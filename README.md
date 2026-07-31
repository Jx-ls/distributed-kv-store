<div align="center">

# Distributed Key-Value Store

*A fault-tolerant distributed key-value store built in C++ using the Raft consensus algorithm for replication, persistence, and horizontal scalability.*

![](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![](https://img.shields.io/badge/Consensus-Raft-blue?style=flat-square)
![](https://img.shields.io/badge/Storage-Key--Value-success?style=flat-square)
![](https://img.shields.io/badge/Replication-Distributed-orange?style=flat-square) <br/>
![](https://img.shields.io/badge/Persistence-WAL%20%26%20Snapshots-8A2BE2?style=flat-square)
![](https://img.shields.io/badge/Sharding-Consistent%20Hashing-green?style=flat-square)
![](https://img.shields.io/badge/Platform-Linux-lightgrey?style=flat-square&logo=linux)
[![](https://img.shields.io/github/license/Jx-ls/distributed-kv-store?style=flat-square)](./LICENSE)

</div>

## Features

* Raft leader election with randomized election timeouts
* Heartbeat protocol for leader liveness and failure detection
* Strongly consistent replicated state machine using majority consensus
* Log replication with automatic follower catch-up
* Log conflict detection and resolution for diverging replicas
* Write-Ahead Logging (WAL) for durable persistence
* Snapshot-based log compaction to reduce recovery overhead
* Crash recovery from persisted WAL and snapshots
* Binary serialization for efficient on-disk storage
* Consistent hash based shard routing across independent Raft clusters
* Horizontal scalability through shard distribution
* Thread-safe asynchronous Raft node implementation
* **Interactive REPL Shell** for long-running cluster interaction

## Project Structure

```text
.
├── bin
│   └── kvstore                  # Executable
├── data                         # Persistent Storage
│   ├── cluster-0                # Shard 0 WAL and Snapshots
│   │   ├── node_0_wal.bin       
│   │   └── ...
│   └── cluster-1                # Shard 1 WAL and Snapshots
│       ├── node_0_wal.bin       
│       └── ...
├── include
│   ├── consistent_hash.h        # Consistent hashing ring
│   ├── log.h                    # Replicated log abstraction
│   ├── message.h                # RPC messages & serialization
│   ├── raft.h                   # Raft node implementation
│   ├── shard_router.h           # Multi-cluster router
│   ├── storage.h                # Key-value storage engine
│   ├── transport.h              # Transport/RPC layer
│   └── wal.h                    # Write-ahead log & snapshots
├── src
│   ├── consistent_hash.cpp
│   ├── log.cpp
│   ├── main.cpp                 # REPL Shell & Cluster bootstrap
│   ├── raft.cpp                 # Core Raft protocol
│   ├── shard_router.cpp
│   ├── storage.cpp
│   ├── transport.cpp            # Message routing
│   └── wal.cpp                  # Persistence & recovery
├── LICENSE
├── Makefile
└── README.md
```

## Building

Clone the repository:

```bash
git clone https://github.com/Jx-ls/distributed-kv-store.git
cd distributed-kv-store
```

Compile the project:

```bash
make
```

This generates the executable:

```text
bin/kvstore
```

To clean generated files:

```bash
make clean
```

## Running

The key-value store now runs as a long-running daemon with an interactive REPL shell. This prevents the high overhead of constantly bootstrapping the cluster for single commands.

Start the interactive shell:
```bash
./bin/kvstore
```

Example session:

```text
$ ./bin/kvstore
====================================================
 Starting Multi-Cluster Sharded Raft KV Store      
====================================================
[System] Waiting for cluster leader elections to stabilize...
[System] Cluster ready! Type 'HELP' for commands.

kvstore> SET Joshua 97
OK (Joshua => 97) [Routed to cluster-0 | Leader Node 1]

kvstore> SET Joseph 42
OK (Joseph => 42) [Routed to cluster-1 | Leader Node 0]

kvstore> GET Joshua
"97" [Routed to cluster-0 | Node 1]

kvstore> GET Joseph
"42" [Routed to cluster-1 | Node 0]

kvstore> EXIT
Shutting down cluster...
```

## Architecture

Each shard consists of an independent Raft cluster responsible for maintaining a replicated key-value state machine. Client requests are routed to the appropriate shard using consistent hashing, where the elected leader coordinates replication across follower replicas.

Every write is first appended to the replicated log and persisted through a Write-Ahead Log (WAL) organized by cluster ID and node ID. Once a majority of replicas acknowledge the entry, it is committed and applied to the storage engine. Periodic snapshots compact the replicated log, allowing nodes to recover efficiently after crashes without replaying the entire history.

Leader election is performed using randomized election timeouts and heartbeat messages. Followers automatically detect leader failures, initiate elections, and continue serving requests after a new leader is chosen, ensuring high availability under node failures. The interactive shell allows users to interface with the active clusters in real-time without needing to restart the servers for every operation.

## Technologies Used

* C++20
* Raft Consensus Algorithm
* Consistent Hashing
* Write-Ahead Logging (WAL)
* Snapshot-Based Persistence
* Binary Serialization
* GNU Make
* Linux

## Concepts Demonstrated

* Distributed systems
* Consensus algorithms (Raft)
* Leader election
* Log replication
* Heartbeat protocols
* Failure detection
* State machine replication
* Write-ahead logging
* Snapshotting
* Crash recovery
* Binary serialization
* Consistent hashing
* Shard routing
* Horizontal scaling
* Fault tolerance
* Concurrent programming
* Distributed key-value storage

## Build Requirements

* Linux
* g++ with C++20 support
* GNU Make

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
