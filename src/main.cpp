#include "../include/WebServer.h"
#include <iostream>

int main() {
    // Run server on Port 8080 with 4 threads
    WebServer server(8080, 4);
    server.start();
    return 0;
}