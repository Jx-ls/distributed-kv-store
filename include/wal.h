#pragma once

#include "message.h"
#include "storage.h"
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct SnapshotMeta {
    int last_included_index{0};
    int last_included_term{0};
};

struct StateMeta {
    int current_term{0};
    int voted_for{-1};
};

// Container returned by Wal
struct RecoveryState {
    SnapshotMeta meta;
    StateMeta state;
    Storage storage;
    vector<LogEntry> uncompacted_entries;
};

class Wal {
private:
    string wal_path;
    string snap_path;
    string state_path;

public:
    Wal(const string& clusterId, int id, string dir = "./data/");

    void append(int index, const LogEntry&);
    void save_snapshot(const SnapshotMeta& meta, Storage& storage);
    void save_state(int current_term, int voted_for);
    RecoveryState recover();
};