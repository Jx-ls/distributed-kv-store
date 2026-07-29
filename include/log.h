#pragma once

#include "message.h"
#include <vector>
using namespace std;

// 1- based indexing, using 0 to indicate no entries
class Log {
public:
    int     lastIncludedIndex{0}; // in snapshot
    int     lastIncludedTerm{0};  // in snapshot

    int     toVectorIndex(int index) const;
    int     append(const LogEntry& entry);
    LogEntry& at(int index);
    const   LogEntry& at(int index) const;
    int     lastIndex() const;
    int     lastTerm() const;
    void    snapshot(int snapshotIndex, int snapshotTerm);

    const std::vector<LogEntry>& get_entries() const {
        return entries;
    }

private:
    vector<LogEntry> entries;
};