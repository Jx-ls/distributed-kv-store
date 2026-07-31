#include "../include/shard_router.h"
#include <memory>

RaftCluster::RaftCluster(const string& clusterId, const std::vector<int>& nodeIds) 
    : clusterId(clusterId) {
    
    // Create Raft nodes for this cluster
    for (int nid : nodeIds) {
        std::vector<int> peers;
        for (int pid : nodeIds) {
            if (pid != nid) peers.push_back(pid);
        }
        nodes.push_back(std::make_unique<RaftNode>(clusterId, nid, peers, &router));
    }

    // Register each node with cluster's transport router
    for (size_t i = 0; i < nodeIds.size(); ++i) {
        router.register_node(nodeIds[i], nodes[i].get());
    }
}

RaftCluster::~RaftCluster() {
    stop();
}

void RaftCluster::start() {
    for (auto& node : nodes) {
        node->start();
    }
}

void RaftCluster::stop() {
    for (auto& node : nodes) {
        node->stop();
    }
}

RaftNode* RaftCluster::get_leader() {
    for (auto& node : nodes) {
        if (node->get_state() == NodeState::LEADER) {
            return node.get();
        }
    }
    return nullptr;
}

ShardRouter::ShardRouter(int replicasPerCluster) : ring(replicasPerCluster) {}

void ShardRouter::add_cluster(const std::string& clusterId, std::shared_ptr<RaftCluster> cluster) {
    clusters[clusterId] = cluster;
    ring.add_cluster(clusterId);
}

void ShardRouter::remove_cluster(const std::string& clusterId) {
    clusters.erase(clusterId);
    ring.remove_cluster(clusterId);
}

bool ShardRouter::execute_set(const std::string& key, const std::string& value, std::string& outClusterId, int& outLeaderId) {
    outClusterId = ring.get_cluster(key);
    if (outClusterId.empty()) return false;

    auto it = clusters.find(outClusterId);
    if (it == clusters.end()) return false;

    RaftNode* leader = it->second->get_leader();
    if (!leader) return false;

    outLeaderId = leader->get_id();
    return leader->execute_client_set(key, value);
}

bool ShardRouter::execute_get(const std::string& key, std::string& value, std::string& outClusterId, int& outLeaderId) {
    outClusterId = ring.get_cluster(key);
    if (outClusterId.empty()) return false;

    auto it = clusters.find(outClusterId);
    if (it == clusters.end()) return false;

    RaftNode* leader = it->second->get_leader();
    if (!leader) return false;

    outLeaderId = leader->get_id();
    return leader->execute_client_get(key, value);
}