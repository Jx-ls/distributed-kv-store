#include <iostream>
#include <cassert>
#include <filesystem>
#include "../include/raft.h" // Includes Storage, Log, WALManager, and Message definitions

void clean_test_directory() {
    std::filesystem::remove_all("./data");
}

int main() {
    clean_test_directory();

    std::cout << " 1. Initializing RaftNode (Node ID: 0)\n";

    int node_id = 0;
    
    // Create first node instance
    {
        RaftNode node(node_id);

        std::cout << "[Step 1] Appending log entries to WAL and memory...\n";
        
        // Append entries
        LogEntry e1{1, "user:101", "Alice"};
        LogEntry e2{1, "user:102", "Bob"};
        LogEntry e3{2, "user:103", "Charlie"};

        node.append_log(e1); // Raft Index 1
        node.append_log(e2); // Raft Index 2
        node.append_log(e3); // Raft Index 3

        std::cout << "[Step 2] Applying entries up to Index 2 to Storage...\n";
        node.apply_to_storage(2); // Commits "user:101" and "user:102"

        // Verify values exist in Storage
        std::string val1, val2;
        assert(node.get_storage().get("user:101", val1) && val1 == "Alice");
        assert(node.get_storage().get("user:102", val2) && val2 == "Bob");
        std::cout << " -> Storage verified: user:101=" << val1 << ", user:102=" << val2 << "\n";

        std::cout << "[Step 3] Taking Snapshot at Index 2, Term 1...\n";
        SnapshotMeta meta{2, 1};
        node.take_snapshot(meta);

        std::cout << "[Step 4] Appending entry past snapshot...\n";
        LogEntry e4{2, "user:104", "David"};
        node.append_log(e4); // Raft Index 4
        node.apply_to_storage(4); // Apply up to Index 4

        std::cout << "\n---> Node 0 crashing / shutting down! <---\n\n";
    } // Node goes out of scope and is destroyed

    std::cout << " 2. Recovering RaftNode from Disk\n";

    // Recover state into a new RaftNode instance
    {
        RaftNode recovered_node(node_id);

        std::cout << "[Check 1] Verifying Snapshot Metadata...\n";
        std::cout << " -> Recovered lastIncludedIndex: " << recovered_node.get_log().lastIncludedIndex << " (Expected: 2)\n";
        std::cout << " -> Recovered lastIncludedTerm:  " << recovered_node.get_log().lastIncludedTerm << " (Expected: 1)\n";
        assert(recovered_node.get_log().lastIncludedIndex == 2);
        assert(recovered_node.get_log().lastIncludedTerm == 1);

        std::cout << "[Check 2] Verifying Storage state...\n";
        std::string val1, val2, val3, val4;
        assert(recovered_node.get_storage().get("user:101", val1) && val1 == "Alice");
        assert(recovered_node.get_storage().get("user:102", val2) && val2 == "Bob");
        assert(recovered_node.get_storage().get("user:103", val3) && val3 == "Charlie");
        assert(recovered_node.get_storage().get("user:104", val4) && val4 == "David");
        
        std::cout << " -> user:101 = " << val1 << " [OK]\n";
        std::cout << " -> user:102 = " << val2 << " [OK]\n";
        std::cout << " -> user:103 = " << val3 << " [OK]\n";
        std::cout << " -> user:104 = " << val4 << " [OK]\n";

        std::cout << "[Check 3] Verifying uncompacted Log entries...\n";
        std::cout << " -> Active memory log count: " << recovered_node.get_log().get_entries().size() << " (Expected: 2 entries: e3 & e4)\n";
        assert(recovered_node.get_log().get_entries().size() == 2);
    }

    std::cout << " SUCCESS: WAL & Snapshot Recovery Test Passed\n";

    return 0;
}