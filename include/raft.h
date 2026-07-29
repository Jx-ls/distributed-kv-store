#pragma once

#include "message.h"
#include "node.h"
#include "log.h"
#include "storage.h"

class RaftNode {
public:
    explicit RaftNode(int id);

    bool commitTo(int toCommit);

    int getCommitIndex() const { return commitIndex; }
    int getLastApplied() const { return lastApplied; }

    int appendEntry(int term, const string& key, const string& value);
    bool get(const string& key, string& value);

private:
    int id;

    int commitIndex = 0;
    int lastApplied = 0;

    Log log;
    Storage storage;
};