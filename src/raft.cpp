#include "../include/raft.h"
#include <algorithm>


RaftNode::RaftNode(int id) : id(id), commitIndex(0), lastApplied(0) {}

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