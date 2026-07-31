#pragma once

#include "message.h"
#include <unordered_map>
#include <mutex>

class ITransport {
public:
    virtual ~ITransport() = default;
    virtual RequestVoteResponse send_request_vote(int target_id, const RequestVoteRequest& req) = 0;
    virtual AppendEntriesResponse send_append_entries(int target_id, const AppendEntriesRequest& req) = 0;
    virtual InstallSnapshotResponse send_install_snapshot(int target_id, const InstallSnapshotRequest& req) = 0;
};

class RaftNode;

// In memory router
class Router : public ITransport {
private:
    unordered_map<int, RaftNode*> nodes;
    mutex router_mutex;

public:
    void register_node(int id, RaftNode* node) {
        lock_guard<mutex> lock(router_mutex);
        nodes[id] = node;
    }
    RequestVoteResponse send_request_vote(int target_id, const RequestVoteRequest& req) override;
    AppendEntriesResponse send_append_entries(int target_id, const AppendEntriesRequest& req) override;
    InstallSnapshotResponse send_install_snapshot(int target_id, const InstallSnapshotRequest& req) override;

};