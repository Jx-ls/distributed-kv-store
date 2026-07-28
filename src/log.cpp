#include "../include/log.h"
using namespace std;

int Log::append(const LogEntry& entry) {
    entries.push_back(entry);
    return entries.size() - 1;
}

LogEntry& Log::at(int index) {
    return entries[index];
}

const LogEntry& Log::at(int index) const {
    return entries[index];
}

int Log::size() const {
    return entries.size();
}