#pragma once

#include <string>
#include <map>

using namespace std;

class ConsistentHashRing {
public:
    explicit ConsistentHashRing(int replicas = 100);

    void add_cluster(const string& clusterId);
    void remove_cluster(const string& clusterId);

    string get_cluster(const string& key) const;

private:
    int num_replicas; // vnodes
    map<int, string> ring;
    
    int hash_fn(const string& key) const;
};