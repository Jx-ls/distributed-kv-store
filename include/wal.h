#pragma once

#include "message.h"
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct SnapshotMeta {
    int last_included_index{0};
    int last_included_term{0};
};

// Container returned by Wal
struct RecoveryState {
    SnapshotMeta meta;
    unordered_map<string, string> kv;
    vector<LogEntry> uncompacted_entries;
};

class Wal {
private:
    string wal_path;
    string snap_path;

public:
    Wal(string path = "./raft_") : wal_path(path + "wal.bin"), snap_path(path + "snap.bin"){}

    void    append(int index, const LogEntry&);
    void    save_snapshot(const SnapshotMeta& meta, const unordered_map<string, string>& kv, const vector<LogEntry>& remaining_logs);
    RecoveryState recover();
};