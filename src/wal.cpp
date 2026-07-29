#include "../include/wal.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

void Wal::append(int index, const LogEntry& entry) {
    ofstream ofs(wal_path, ios::binary | ios::app);
    Serializer::writeLogEntry(ofs, index, entry);
}

void Wal::save_snapshot(const SnapshotMeta& meta, const unordered_map<string, string>& kv, const vector<LogEntry>& remaining_logs) {
    ofstream snap_ofs(snap_path, ios::binary | ios::trunc);
    Serializer::writeVal(snap_ofs, meta.last_included_index);
    Serializer::writeVal(snap_ofs, meta.last_included_term);

    uint32_t count = kv.size();
    Serializer::writeVal(snap_ofs, count);
    for (const auto& [k, v] : kv) {
        Serializer::writeVal(snap_ofs, k);
        Serializer::writeVal(snap_ofs, v);
    }

    ofstream wal_ofs(wal_path, ios::binary | ios::trunc);
    int current_idx = meta.last_included_index + 1;
    for (const auto& entry : remaining_logs) {
        Serializer::writeLogEntry(wal_ofs, current_idx++, entry);
    }
}

RecoveryState Wal::recover() {
    RecoveryState state;

    // load snapshot if present
    ifstream snap_ifs(snap_path, ios::binary);
    if (snap_ifs.is_open()) {
        Serializer::readVal(snap_ifs, state.meta.last_included_index);
        Serializer::readVal(snap_ifs, state.meta.last_included_term);

        uint32_t count = 0;
        Serializer::readVal(snap_ifs, count);

        for (uint32_t i = 0; i < count; i++) {
            string k, v;
            Serializer::readString(snap_ifs, k);
            Serializer::readString(snap_ifs, v);
            state.kv[k] = v;
        }
    }

    // replay uncompacted log entries
    ifstream wal_ifs(wal_path, ios::binary);
    if (wal_ifs.is_open()) {
        int index = 0;
        LogEntry entry;
        while (Serializer::readLogEntry(wal_ifs, index, entry)) {
            if (index > state.meta.last_included_index) {
                state.uncompacted_entries.push_back(entry);
                state.kv[entry.key] = entry.value;
            }
        }
    }
    return state;
}