#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

struct LogEntry {
    int term;
    string type;
    string key;
    string value;
};

struct RequestVoteRequest {
    int term;
    int candidateId;

    int lastLogIndex;
    int lastLogTerm;
};

struct RequestVoteResponse {
    int term;
    bool voteGranted;
};

struct AppendEntriesRequest {
    int term;
    int leaderId;

    int prevLogIndex;
    int prevLogTerm;

    vector<LogEntry> entries;
    
    int leaderCommit;
};

struct AppendEntriesResponse {
    int term;
    bool success;

    int matchIndex;
};

struct InstallSnapshotRequest {
    int term;
    int leaderId;
    int lastIncludedIndex;
    int lastIncludedTerm;
    std::vector<std::pair<std::string, std::string>> state_data; 
};

struct InstallSnapshotResponse {
    int term;
};

namespace Serializer {
    template <typename T>
    inline void writeVal(ostream& os, const T& val) {
        os.write(reinterpret_cast<const char*>(&val), sizeof(T));
    }

    template <typename T>
    inline bool readVal(istream& is, T& val) {
        return static_cast<bool>(is.read(reinterpret_cast<char*>(&val), sizeof(T)));
    }

    inline void writeString(ostream& os, const string& str) {
        uint32_t len = str.size();
        writeVal(os, len); // header 4-byte
        os.write(str.data(), len); // data
    }

    inline bool readString(istream& is, string& str) {
        uint32_t len = 0;
        if (!readVal(is, len)) return false;
        str.resize(len); // resizes str by reading header
        return static_cast<bool>(is.read(&str[0], len));
    }

    /* Log Entry rw */
    inline void writeLogEntry(ostream& os, int index, const LogEntry& entry) {
        writeVal(os, index);
        writeVal(os, entry.term);
        writeString(os, entry.type);
        writeString(os, entry.key);
        writeString(os, entry.value);
    }

    inline bool readLogEntry(istream& is, int& index, LogEntry& entry) {
        if (!readVal(is, index)) return false;
        if (!readVal(is, entry.term)) return false;
        if (!readString(is, entry.type)) return false;
        if (!readString(is, entry.key)) return false;
        return readString(is, entry.value);
    }
}