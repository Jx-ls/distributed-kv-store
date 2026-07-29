#include "../include/raft.h"
#include <algorithm>


RaftNode::RaftNode(int id) : id(id), commitIndex(0), lastApplied(0), wal(id) {
    RecoveryState state = wal.recover();

    storage = state.storage;
    log.lastIncludedIndex = state.meta.last_included_index;
    log.lastIncludedTerm = state.meta.last_included_term;

    // replay uncompacted entries
    for (const auto& entry : state.uncompacted_entries) {
        log.append(entry);
    }
}

bool RaftNode::commitTo(int toCommit) {
    if (toCommit >= commitIndex) {
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

int RaftNode::appendEntry(int term, const string& key, const string& value) {
    LogEntry entry{term, key, value};
    return log.append(entry);
}

bool RaftNode::get(const string& key, string& value) {
    return storage.get(key, value);
}