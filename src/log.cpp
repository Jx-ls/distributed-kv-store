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
    if (index < 1) {
        throw out_of_range("Log::at: index must be >= 1 (received " + to_string(index) + ")");
    }
    int vector_offset = toVectorIndex(index);
    if (vector_offset < 0 || vector_offset >= static_cast<int>(entries.size())) {
        throw out_of_range("Log::at: index " + to_string(index) + 
                           " out of range (lastIncludedIndex=" + to_string(lastIncludedIndex) + 
                           ", size=" + to_string(entries.size()) + ")");
    }
    return entries.at(vector_offset);
}

const LogEntry& Log::at(int index) const {
    int vector_offset = toVectorIndex(index);
    if (vector_offset < 0 || vector_offset >= static_cast<int>(entries.size())) {
        throw out_of_range("Log::at: index out of range (" + to_string(index) + ")");
    }
    return entries.at(vector_offset);
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

void Log::truncate_from(int index) {
    if (index <= lastIncludedIndex) return;

    // Calculate offset in the zero-indexed entries vector
    int vector_offset = index - (lastIncludedIndex + 1);

    if (vector_offset >= 0 && vector_offset < static_cast<int>(entries.size())) {
        entries.erase(entries.begin() + vector_offset, entries.end());
    }
}
