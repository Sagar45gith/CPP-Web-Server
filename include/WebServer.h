#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "ThreadPool.h"
#include <netinet/in.h>
#include <string>

class WebServer {
public:
    WebServer(int port, int thread_pool_size);
    void start(); // Start the main loop

private:
    int port;
    int server_fd;
    ThreadPool thread_pool;

    void handle_client(int client_socket);
    std::string load_file(const std::string& path);
    std::string get_mime_type(const std::string& path);
};

#endif