#include "../include/transport.h"
#include "../include/raft.h"

RequestVoteResponse Router::send_request_vote(int target_id, const RequestVoteRequest& req) {
    RaftNode* target = nullptr;
    {
        std::lock_guard<std::mutex> lock(router_mutex);
        auto it = nodes.find(target_id);
        if (it != nodes.end()) target = it->second;
    }
    if (target) return target->handle_request_vote(req);
    return RequestVoteResponse{.term = req.term, .voteGranted = false};
}

AppendEntriesResponse Router::send_append_entries(int target_id, const AppendEntriesRequest& req) {
    RaftNode* target = nullptr;
    {
        std::lock_guard<std::mutex> lock(router_mutex);
        auto it = nodes.find(target_id);
        if (it != nodes.end()) target = it->second;
    }
    if (target) return target->handle_append_entries(req);
    return AppendEntriesResponse{.term = req.term, .success = false, .matchIndex = 0};
}
