#pragma once

#include "consistent_hash.h"
#include "transport.h"
#include "raft.h"
#include <memory>
#include <string>
#include <unordered_map>

class RaftCluster {
public:
    RaftCluster(const string& clusterId, const vector<int>& nodeIds);
    ~RaftCluster();

    void start();
    void stop();

    RaftNode* get_leader();
    string get_id() const { return clusterId; };
private:
    string clusterId;
    Router router;
    vector<unique_ptr<RaftNode>> nodes;
};

class ShardRouter {
public:
    explicit ShardRouter(int replicasPerCluster = 100);

    void add_cluster(const string& clusterId, shared_ptr<RaftCluster> cluster);
    void remove_cluster(const string& clusterId);

    bool execute_set(const string& key, const string& value, string& outClusterId, int& outLeaderId);
    bool execute_get(const string& key, string& value, string& outClusterId, int& outLeaderId);

private:
    ConsistentHashRing ring;
    unordered_map<string, shared_ptr<RaftCluster>> clusters;
};