#pragma once

#include "message.h"
#include <vector>
using namespace std;

// 1- based indexing, using 0 to indicate no entries
class Log {
public:
    int append(const LogEntry& entry);
    LogEntry& at(int index);
    const LogEntry& at(int index) const;
    
    int lastIndex() const;
    int lastTerm() const;

    void eraseFrom(int index);

private:
    vector<LogEntry> entries;
};