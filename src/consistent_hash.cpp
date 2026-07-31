#include "../include/consistent_hash.h"
#include <string>

ConsistentHashRing::ConsistentHashRing(int replicas) : num_replicas(replicas) {}

int ConsistentHashRing::hash_fn(const string& key) const {
    return static_cast<int>(hash<string>{}(key));
}

void ConsistentHashRing::add_cluster(const string& clusterId) {
    for (int i = 0; i < num_replicas; i++) {
        string vnode_key = clusterId + "#vnode" + to_string(i);
        int hash = hash_fn(vnode_key);
        ring[hash] = clusterId;
    }
}

void ConsistentHashRing::remove_cluster(const string& clusterId) {
    for (int i = 0; i < num_replicas; i++) {
        string vnode_key = clusterId + "#vnode" + to_string(i);
        int hash = hash_fn(vnode_key);
        ring.erase(hash);
    }
}

string ConsistentHashRing::get_cluster(const string& key) const {
    if (ring.empty()) {
        return "";
    }

    int hash = hash_fn(key);
    auto it = ring.lower_bound(hash);

    if (it == ring.end()) {
        it = ring.begin();
    }
    return it -> second;
}