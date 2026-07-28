#pragma once

#include <string>
#include <vector>
using namespace std;

struct LogEntry {
    int term;
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