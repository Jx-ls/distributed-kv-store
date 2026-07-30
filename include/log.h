#pragma once

#include "message.h"
#include <vector>
using namespace std;

// 1- based indexing, using 0 to indicate no entries
class Log {
public:
    int lastIncludedIndex{0}; // in snapshot
    int lastIncludedTerm{0};  // in snapshot

    int toVectorIndex(int index) const;
    int append(const LogEntry& entry);
    LogEntry& at(int index);
    const LogEntry& at(int index) const;
    size_t size() { return entries.size(); }
    int lastIndex() const;
    int lastTerm() const;
    void snapshot(int snapshotIndex, int snapshotTerm);

private:
    vector<LogEntry> entries;
};