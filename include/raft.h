#pragma once

#include "message.h"
#include "log.h"
#include "storage.h"
#include "transport.h"
#include "wal.h"
#include <chrono>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>

enum class NodeState {
    FOLLOWER,
    CANDIDATE,
    LEADER
};

class RaftNode {
public:
    RaftNode(int id, vector<int> peer_ids, ITransport* transport);
    ~RaftNode();

    // async thread lifecycle
    void start();
    void stop();

    // client operations
    bool execute_client_set(const string& key, const string& value);
    bool execute_client_get(const string& key, string& value);

    // RPC Handlers
    RequestVoteResponse handle_request_vote(const RequestVoteRequest& req);
    AppendEntriesResponse handle_append_entries(const AppendEntriesRequest& req);

    int get_id() const;
    NodeState get_state() const;
    int get_current_term() const;
    int get_voted_for() const;


private:
    int id;
    vector<int> peers;
    ITransport* transport;

    int currentTerm = 0;
    int votedFor = -1; // -1: voted for no one

    int commitIndex = 0;
    int lastApplied = 0;
    bool commitTo(int toCommit);
    NodeState state = NodeState::FOLLOWER;

    std::chrono::steady_clock::time_point last_heartbeat;
    int election_timeout_ms;

    Log log;
    Storage storage;
    Wal wal;

    mutable mutex node_mutex;
    atomic<bool> running{false};
    thread worker_thread;

    void run_loop();
    void reset_election_timer();
    void start_election();
    void send_heartbeats();

};