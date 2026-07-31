#include "../include/wal.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

Wal::Wal(const string& clusterId, int id, string dir) {
    string cluster_dir = dir + "/" + clusterId + "/";
    filesystem::create_directories(cluster_dir);

    string prefix = cluster_dir + "node_" + to_string(id) + "_";
    wal_path = prefix + "wal.bin";
    snap_path = prefix + "snap.bin";
    state_path = prefix + "state.bin";
}

void Wal::append(int index, const LogEntry& entry) {
    ofstream ofs(wal_path, ios::binary | ios::app);
    Serializer::writeLogEntry(ofs, index, entry);
}

void Wal::save_snapshot(const SnapshotMeta& meta, Storage& storage) {
    ofstream snap_ofs(snap_path, ios::binary | ios::trunc);
    Serializer::writeVal(snap_ofs, meta.last_included_index);
    Serializer::writeVal(snap_ofs, meta.last_included_term);

    vector<string> keys = storage.get_all_keys();
    uint32_t count = keys.size();
    Serializer::writeVal(snap_ofs, count);
    for (const auto& k : keys) {
        string v;
        storage.get(k, v);
        Serializer::writeString(snap_ofs, k);
        Serializer::writeString(snap_ofs, v);
    }
}

void Wal::save_state(int current_term, int voted_for) {
    ofstream ofs(state_path, ios::binary | ios::trunc);
    Serializer::writeVal(ofs, current_term);
    Serializer::writeVal(ofs, voted_for);
    ofs.flush(); // Ensure it hits disk immediately
}

RecoveryState Wal::recover() {
    RecoveryState state;

    ifstream state_ifs(state_path, ios::binary);
    if (state_ifs.is_open()) {
        Serializer::readVal(state_ifs, state.state.current_term);
        Serializer::readVal(state_ifs, state.state.voted_for);
    }

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
            state.storage.put(k, v);
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
                if (!entry.key.empty()) {
                    state.storage.put(entry.key, entry.value);
                }
            }
        }
    }
    return state;
}