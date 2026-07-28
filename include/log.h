#pragma once

#include "message.h"
#include <vector>
using namespace std;

class Log {
public:
    int append(const LogEntry& entry);
    LogEntry& at(int index);
    const LogEntry& at(int index) const;
    int size() const;

private:
    vector<LogEntry> entries;
};