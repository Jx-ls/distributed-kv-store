#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include "../include/raft.h"
#include "../include/transport.h"

using namespace std;

void print_usage() {
    cout << "Usage:\n";
    cout << "  ./kvstore SET <key> <value>\n";
    cout << "  ./kvstore GET <key>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    string command = argv[1];

    cout << "====================================================\n";
    cout << " Starting 3-Node Raft Cluster with Async Elections  \n";
    cout << "====================================================\n";

    Router router;

    // Spin up 3 nodes as separate threads
    RaftNode node0(0, {1, 2}, &router);
    RaftNode node1(1, {0, 2}, &router);
    RaftNode node2(2, {0, 1}, &router);

    router.register_node(0, &node0);
    router.register_node(1, &node1);
    router.register_node(2, &node2);

    // Start background election timer loops
    node0.start();
    node1.start();
    node2.start();

    // Allow background threads time to trigger election and discover leader
    cout << "[Cluster] Waiting for dynamic election to resolve leader...\n";
    this_thread::sleep_for(chrono::milliseconds(500));

    // Discover elected Leader dynamically
    vector<RaftNode*> cluster = {&node0, &node1, &node2};
    RaftNode* leader = nullptr;

    for (RaftNode* n : cluster) {
        if (n->get_state() == NodeState::LEADER) {
            leader = n;
            break;
        }
    }

    if (!leader) {
        cout << "Error: No leader was elected (Split vote). Try again.\n";
        return 1;
    }

    cout << "[Client] Cluster Active. Target Leader is Node " << leader->get_id() << "\n\n";

    if (command == "SET" || command == "PUT") {
        if (argc < 4) {
            print_usage();
            return 1;
        }
        string key = argv[2];
        string val = argv[3];

        bool success = leader->execute_client_set(key, val);
        if (success) {
            cout << "OK (" << key << " => " << val << ") [Written via Leader " << leader->get_id() << "]\n";
        } else {
            cout << "ERROR: Failed to write command to Leader.\n";
        }

    } else if (command == "GET") {
        if (argc < 3) {
            print_usage();
            return 1;
        }
        string key = argv[2];
        string value;
        if (!leader->execute_client_get(key, value)) {
            cout << "(nil)\n";
        } else {
            cout << "\"" << value << "\"\n";
        }
    } else {
        print_usage();
    }

    // Clean shutdown of worker threads
    node0.stop();
    node1.stop();
    node2.stop();

    return 0;
}