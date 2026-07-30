#include "../include/raft.h"
#include <algorithm>
#include <chrono>
#include <mutex>
#include <random>
#include <thread>


RaftNode::RaftNode(int id, vector<int> peer_ids, ITransport* transport) : id(id), peers(peer_ids), transport(transport), commitIndex(0), lastApplied(0), wal(id) {
    RecoveryState state = wal.recover();

    storage = state.storage;
    log.lastIncludedIndex = state.meta.last_included_index;
    log.lastIncludedTerm = state.meta.last_included_term;

    // replay uncompacted entries
    for (const auto& entry : state.uncompacted_entries) {
        log.append(entry);
    }

    reset_election_timer();
}

RaftNode::~RaftNode() {
    stop();
}

void RaftNode::start() {
    running = true;
    worker_thread = std::thread(&RaftNode::run_loop, this);
}

void RaftNode::stop() {
    running = false;
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void RaftNode::run_loop() {
    while (running) {
        this_thread::sleep_for(chrono::milliseconds(20));
        lock_guard<mutex> lock(node_mutex);

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_heartbeat).count();

        if (state == NodeState::LEADER) {
            if (elapsed >= 50) { // Broadcast heartbeats every 50ms
                send_heartbeats();
                last_heartbeat = std::chrono::steady_clock::now();
            }
        } else {
            if (elapsed >= election_timeout_ms) {
                start_election();
            }
        }
    }
}

void RaftNode::reset_election_timer() {
    last_heartbeat = chrono::steady_clock::now();

    // random time between 150ms and 300ms
    static mt19937 gen(id + chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<> dist(150, 300);
    election_timeout_ms = dist(gen);
}

void RaftNode::start_election() {
    state = NodeState::CANDIDATE;
    currentTerm++;
    votedFor = id;
    reset_election_timer();

    int votes_granted = 1;
    int quorum = ((peers.size() + 1)/ 2) + 1; // majority formula

    RequestVoteRequest req{
        .term = currentTerm,
        .candidateId = id,
        .lastLogIndex = log.lastIndex(),
        .lastLogTerm = log.lastTerm()
    };

    for (int peer_id : peers) {
        RequestVoteResponse res = transport -> send_request_vote(peer_id, req);

        if (res.term > currentTerm) {
            currentTerm = res.term;
            state = NodeState::FOLLOWER;
            votedFor = -1;
            return;
        }

        if (res.voteGranted) {
            votes_granted++;
        }
    }

    if (votes_granted >= quorum) {
        state = NodeState::LEADER;
        send_heartbeats();
    } else {
        state = NodeState::FOLLOWER;
    }
}

void RaftNode::send_heartbeats() {
    if (state != NodeState::LEADER) return;

    for (int peer_id : peers) {
        int prev_idx = nextIndex[peer_id] - 1;
        int prev_term = 0;

        if (prev_idx > 0) {
            if (prev_idx == log.lastIncludedIndex) {
                prev_term = log.lastIncludedTerm;
            } else if (prev_idx > log.lastIncludedIndex) {
                prev_term = log.at(prev_idx).term;
            }
        }

        vector<LogEntry> entries_to_send;
        for (int i = nextIndex[peer_id]; i <= log.lastIndex(); ++i) {
            if (i > log.lastIncludedIndex) {
                entries_to_send.push_back(log.at(i));
            }
        }

        AppendEntriesRequest req{
            .term = currentTerm,
            .leaderId = id,
            .prevLogIndex = prev_idx,
            .prevLogTerm = prev_term,
            .entries = entries_to_send,
            .leaderCommit = commitIndex
        };

        AppendEntriesResponse res = transport->send_append_entries(peer_id, req);

        if (res.term > currentTerm) {
            currentTerm = res.term;
            state = NodeState::FOLLOWER;
            votedFor = -1;
            return;
        }

        if (res.success) {
            matchIndex[peer_id] = res.matchIndex;
            nextIndex[peer_id] = res.matchIndex + 1;
        } else if (nextIndex[peer_id] > 1) {
            nextIndex[peer_id]--; // Step back to resolve log divergence
        }
    }

    // Advance commitIndex when entry is replicated on a majority of nodes
    int quorum = ((peers.size() + 1) / 2) + 1;
    for (int N = log.lastIndex(); N > commitIndex; --N) {
        if (N <= log.lastIncludedIndex) break;
        if (log.at(N).term != currentTerm) continue;

        int count = 1; // Self count
        for (int peer_id : peers) {
            if (matchIndex[peer_id] >= N) count++;
        }

        if (count >= quorum) {
            commitTo(N);
            break;
        }
    }
}

bool RaftNode::execute_client_set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(node_mutex);
    if (state != NodeState::LEADER) {
        return false;
    }

    LogEntry entry{
        .term = currentTerm,
        .type = "SET",
        .key = key,
        .value = value
    };
    int index = log.append(entry);
    wal.append(index, entry);

    send_heartbeats();

    // 3. Trigger Snapshot if threshold is reached
    const size_t SNAPSHOT_THRESHOLD = 1000;
    if (log.size() >= SNAPSHOT_THRESHOLD) {
        log.snapshot(index, currentTerm);
        wal.save_snapshot({index, currentTerm}, storage);
    }
    return true;
}

bool RaftNode::execute_client_get(const string& key, string& value) {
    lock_guard<mutex> lock(node_mutex);
    return storage.get(key, value);
}

RequestVoteResponse RaftNode::handle_request_vote(const RequestVoteRequest& req) {
    RequestVoteResponse res{
        .term = currentTerm,
        .voteGranted = false
    };

    // reject if requester's term is older
    if (req.term < currentTerm) return res;

    if (req.term > currentTerm) {
        currentTerm = req.term;
        state = NodeState::FOLLOWER;
        votedFor = -1;
    }

    bool log_is_up_to_date = false;
    if (req.lastLogTerm > log.lastTerm()) {
        log_is_up_to_date = true;
    } else if (req.lastLogTerm == log.lastTerm() && req.lastLogIndex >= log.lastIndex()) {
        log_is_up_to_date = true;
    }

    if ((votedFor == -1 || votedFor == req.candidateId) && log_is_up_to_date) {
        votedFor = req.candidateId;
        res.voteGranted = true;
        reset_election_timer();
    }
    return res;
}

AppendEntriesResponse RaftNode::handle_append_entries(const AppendEntriesRequest& req) {
    AppendEntriesResponse res{
        .term = currentTerm,
        .success = false,
        .matchIndex = 0
    };

    if (req.term < currentTerm) return res;

    if (req.term >= currentTerm) {
        currentTerm = req.term;
        state = NodeState::FOLLOWER;
        votedFor = -1;
    }

    reset_election_timer();

    if (req.prevLogIndex > 0) {
        if (req.prevLogIndex > log.lastIndex()) {
            return res;
        }

        int actual_prev_term = 0;
        if (req.prevLogIndex == log.lastIncludedIndex) {
            actual_prev_term = log.lastIncludedTerm;
        } else if (req.prevLogIndex > log.lastIncludedIndex) {
            actual_prev_term = log.at(req.prevLogIndex).term;
        }

        if (actual_prev_term != req.prevLogTerm) {
            return res;
        }
    }

    for (size_t i = 0; i < req.entries.size(); ++i) {
        int idx = req.prevLogIndex + 1 + i;
        if (idx <= log.lastIndex()) {
            if (idx > log.lastIncludedIndex && log.at(idx).term != req.entries[i].term) {
                log.truncate_from(idx);
                log.append(req.entries[i]);
                wal.append(idx, req.entries[i]);
            }
        } else {
            log.append(req.entries[i]);
            wal.append(idx, req.entries[i]);
        }
    }

    if (req.leaderCommit > commitIndex) {
        commitTo(min(req.leaderCommit, log.lastIndex()));
    }

    res.success = true;
    res.matchIndex = log.lastIndex();
    return res;
}

bool RaftNode::commitTo(int toCommit) {
    if (toCommit <= commitIndex) {
        return false;
    }
    int targetCommit = min(toCommit, log.lastIndex());
    if (targetCommit <= commitIndex) {
        return false;
    }

    commitIndex = targetCommit;

    while (lastApplied < commitIndex) {
        lastApplied++;
        const LogEntry& entry = log.at(lastApplied);

        if (!entry.key.empty()) {
            storage.put(entry.key, entry.value);
        }
    }

    return true;
}

int RaftNode::get_id() const {
    lock_guard<mutex> lock(node_mutex);
    return id;
}

NodeState RaftNode::get_state() const {
    lock_guard<mutex> lock(node_mutex);
    return state;
}

int RaftNode::get_current_term() const {
    lock_guard<mutex> lock(node_mutex);
    return currentTerm;
}

int RaftNode::get_voted_for() const {
    lock_guard<mutex> lock(node_mutex);
    return votedFor;
}