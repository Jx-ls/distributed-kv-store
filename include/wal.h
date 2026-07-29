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

// Container returned by Wal
struct RecoveryState {
    SnapshotMeta meta;
    Storage storage;
    vector<LogEntry> uncompacted_entries;
};

class Wal {
private:
    string wal_path;
    string snap_path;

public:
    Wal(int id, string dir = "./data/");

    void    append(int index, const LogEntry&);
    void    save_snapshot(const SnapshotMeta& meta,
                          Storage& storage, 
                          const vector<LogEntry>& remaining_logs);
    RecoveryState recover();
};