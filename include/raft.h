#pragma once

#include "message.h"
#include "node.h"
#include "log.h"
#include "storage.h"
#include "wal.h"

class RaftNode {
public:
    explicit RaftNode(int id);

    bool commitTo(int toCommit);

    int getCommitIndex() const { return commitIndex; }
    int getLastApplied() const { return lastApplied; }

    int appendEntry(int term, const string& key, const string& value);
    bool get(const string& key, string& value);

    // Helper: Appends entry to memory Log and persists to WAL
    int append_log(const LogEntry& entry) {
        int idx = log.append(entry);
        wal.append(idx, entry);
        return idx;
    }

    // Helper: Executes entries into Storage up to commit_idx
    void apply_to_storage(int commit_idx) {
        for (int i = log.lastIncludedIndex + 1; i <= commit_idx; ++i) {
            try {
                const LogEntry& entry = log.at(i);
                storage.put(entry.key, entry.value);
            } catch (...) {
                break;
            }
        }
    }

    // Helper: Compacts log vector and saves snapshot to disk
    void take_snapshot(const SnapshotMeta& meta) {
        log.snapshot(meta.last_included_index, meta.last_included_term);
        wal.save_snapshot(meta, storage, log.get_entries());
    }

    // Accessors for testing
    Storage& get_storage() { return storage; }
    Log& get_log() { return log; }

private:
    int id;

    int commitIndex = 0;
    int lastApplied = 0;

    Log log;
    Storage storage;
    Wal wal;
};