#include "../include/log.h"
#include <stdexcept>
using namespace std;

int Log::toVectorIndex(int index) const {
    return index - lastIncludedIndex - 1;
}

int Log::append(const LogEntry& entry) {
    entries.push_back(entry);
    return lastIndex();
}

LogEntry& Log::at(int index) {
    if (index < 1 || index > lastIndex()) {
        throw out_of_range("Log::at: index out of range");
    }
    return entries.at(toVectorIndex(index));
}

const LogEntry& Log::at(int index) const {
    if (index < 1 || index > lastIndex()) {
        throw out_of_range("Log::at: index out of range");
    }
    return entries.at(toVectorIndex(index));
}

int Log::lastIndex() const {
    return lastIncludedIndex + static_cast<int>(entries.size());
}

int Log::lastTerm() const {
    if (entries.empty()) {
        return lastIncludedTerm;
    }
    return entries.back().term;
}

void Log::snapshot(int snapshotIndex, int snapshotTerm) {
    if (snapshotIndex <= lastIncludedIndex) return;
    
    int numEntriesToDrop = snapshotIndex - lastIncludedIndex;
    if (numEntriesToDrop < static_cast<int>(entries.size())) {
        entries.erase(entries.begin(), entries.begin() + numEntriesToDrop);
    } else {
        entries.clear();
    }

    lastIncludedIndex = snapshotIndex;
    lastIncludedTerm = snapshotTerm;
}
