#include "../include/log.h"
#include <stdexcept>
using namespace std;

int Log::append(const LogEntry& entry) {
    entries.push_back(entry);
    return static_cast<int>(entries.size());
}

LogEntry& Log::at(int index) {
    if (index < 1 || index > static_cast<int>(entries.size())) {
        throw out_of_range("Log::at: index out of range");
    }
    return entries[index-1];
}

const LogEntry& Log::at(int index) const {
    if (index < 1 || index > static_cast<int>(entries.size())) {
        throw out_of_range("Log::at: index out of range");
    }
    return entries[index-1];
}

int Log::lastIndex() const {
    return static_cast<int>(entries.size());
}

int Log::lastTerm() const {
    if (entries.empty()) {
        return 0;
    }
    return entries.back().term;
}

void Log::eraseFrom(int index) {
    if (index < 1) {
        entries.clear();
        return;
    }
    if (index <= static_cast<int>(entries.size())) {
        entries.erase(entries.begin() + (index-1), entries.end());
    }
}
