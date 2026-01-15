# High-Performance Multi-Threaded Web Server (C++)

A lightweight, high-throughput HTTP web server built from scratch in C++ using POSIX sockets. Designed to handle high concurrency through a custom Thread Pool architecture, bypassing the overhead of external frameworks.

![C++](https://img.shields.io/badge/Language-C++17-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%2F%20WSL-orange.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

## 🚀 Key Features
- **Custom Thread Pool:** Pre-allocated worker threads prevent the overhead of creating/destroying threads per request.
- **Thread Safety:** Implements `std::mutex` and `std::condition_variable` to manage the task queue without race conditions.
- **Zero-Copy Architecture:** Uses pointers and references to minimize memory overhead during request handling.
- **Security:** Includes path traversal sanitization to prevent unauthorized file access.
- **MIME Type Support:** Serves HTML, CSS, JS, and Images (PNG/JPG) with correct content headers.

## 📊 Performance Benchmarks
Tested using **Apache Benchmark (ab)** on a standard laptop (WSL2 environment):

| Metric | Result |
|--------|--------|
| **Requests Per Second** | **~1,459 req/s** |
| **Concurrency Level** | 100 simultaneous users |
| **Failed Requests** | **0** (0.00% Failure Rate) |
| **Mean Time Per Request** | 0.685 ms |

*> "This server handles 1.5k requests per second without breaking a sweat, proving the efficiency of low-level systems programming."*

## 🛠️ Architecture
The project follows a modular design:
1.  **Server (Listener):** Binds to port 8080 and listens for incoming TCP connections.
2.  **Dispatcher (Main Thread):** Accepts connections and immediately pushes the socket descriptor to the Task Queue.
3.  **Thread Pool (Workers):** A fixed pool of threads (default: 4) sleeps on a Condition Variable. When a task arrives, one thread wakes up, handles the HTTP request, and goes back to sleep.

## 💻 How to Run
### Prerequisites
- GCC Compiler (g++)
- Make
- Linux or WSL (Windows Subsystem for Linux)

### Build & Start
```bash
# 1. Compile the project
make

# 2. Run the server
./server