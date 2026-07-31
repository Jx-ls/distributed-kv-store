#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include "../include/shard_router.h"

using namespace std;

void print_usage() {
    cout << "Commands:\n";
    cout << "   SET <key> <value>\n";
    cout << "   GET <key>\n";
    cout << "   EXIT (or QUIT)\n";
}

int main(int argc, char* argv[]) {
    cout << "====================================================\n";
    cout << " Starting Multi-Cluster Sharded Raft KV Store      \n";
    cout << "====================================================\n";

    // 1. Instantiate 2 Raft clusters with 3 nodes each
    auto cluster0 = std::make_shared<RaftCluster>("cluster-0", std::vector<int>{0, 1, 2});
    auto cluster1 = std::make_shared<RaftCluster>("cluster-1", std::vector<int>{0, 1, 2});

    // 2. Setup ShardRouter with consistent hashing
    ShardRouter shard_router(100);
    shard_router.add_cluster("cluster-0", cluster0);
    shard_router.add_cluster("cluster-1", cluster1);

    // 3. Start nodes and wait for election resolution across clusters
    cluster0->start();
    cluster1->start();

    cout << "[System] Waiting for cluster leader elections to stabilize...\n";
    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[System] Cluster ready! Type 'HELP' for commands.\n\n";

    // 4. Interactive Shell Loop
    string line;
    while (true) {
        cout << "kvstore> ";
        if (!getline(cin, line)) break; // Handle EOF (Ctrl+D)

        if (line.empty()) continue;

        istringstream iss(line);
        string command;
        iss >> command;

        // Convert command to uppercase for case-insensitivity
        for (auto &c : command) c = toupper(c);

        if (command == "EXIT" || command == "QUIT") {
            cout << "Shutting down cluster...\n";
            break;
        } else if (command == "HELP") {
            print_usage();
        } else if (command == "SET" || command == "PUT") {
            string key, val;
            if (iss >> key >> val) {
                string cluster_id;
                int leader_id = -1;
                bool ok = shard_router.execute_set(key, val, cluster_id, leader_id);

                if (ok) {
                    cout << "OK (" << key << " => " << val << ") [Routed to " << cluster_id << " | Leader Node " << leader_id << "]\n";
                } else {
                    cout << "ERROR: Write failed or leader uncontactable on cluster for key: " << key << "\n";
                }
            } else {
                cout << "Usage: SET <key> <value>\n";
            }
        } else if (command == "GET") {
            string key;
            if (iss >> key) {
                string val;
                string cluster_id;
                int leader_id = -1;
                bool ok = shard_router.execute_get(key, val, cluster_id, leader_id);

                if (!ok) {
                    cout << "(nil) [Routed to " << cluster_id << "]\n";
                } else {
                    cout << "\"" << val << "\" [Routed to " << cluster_id << " | Node " << leader_id << "]\n";
                }
            } else {
                cout << "Usage: GET <key>\n";
            }
        } else {
            cout << "Unknown command: " << command << "\n";
        }
    }

    // 5. Graceful teardown
    cluster0->stop();
    cluster1->stop();

    return 0;
}