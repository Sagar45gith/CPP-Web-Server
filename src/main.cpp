#include "../include/WebServer.h"
#include <iostream>
#include <csignal> // Needed for signal handling

// Global pointer to server so the handler can access it
WebServer* server_ptr = nullptr;

// This function runs when you press Ctrl+C
void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    if (server_ptr) {
        std::cout << "Stopping server cleanly...\n";
        server_ptr->stop(); 
    }
    exit(signum); // Now exit the program
}

int main() {
    // 1. Register the signal handler
    signal(SIGINT, signal_handler);

    // 2. Create Server
    WebServer server(8080, 4);
    server_ptr = &server;
    
    // 3. Start Server
    server.start(); 

    return 0;
}