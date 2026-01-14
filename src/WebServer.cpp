#include "../include/WebServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>

const int BUFFER_SIZE = 4096;

WebServer::WebServer(int port, int thread_pool_size) 
    : port(port), thread_pool(thread_pool_size) {
    
    // 1. Setup Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set Options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. Bind
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

void WebServer::start() {
    std::cout << "Server started on port " << port << "...\n";
    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Add task to thread pool
        thread_pool.enqueue([this, new_socket] {
            this->handle_client(new_socket);
        });
    }
}

std::string WebServer::get_mime_type(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css")  != std::string::npos) return "text/css";
    if (path.find(".png")  != std::string::npos) return "image/png";
    if (path.find(".jpg")  != std::string::npos) return "image/jpeg";
    return "text/plain";
}

std::string WebServer::load_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void WebServer::handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE - 1);

    std::istringstream request_stream(buffer);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    if (path == "/") path = "/index.html";
    std::string filepath = "www" + path;
    
    std::string content = load_file(filepath);
    std::string response;

    if (!content.empty()) {
        std::string mime_type = get_mime_type(filepath);
        response = "HTTP/1.1 200 OK\r\nContent-Type: " + mime_type + 
                   "\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n" + content;
    } else {
        std::string error_msg = "404 Not Found";
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: " + 
                   std::to_string(error_msg.size()) + "\r\n\r\n" + error_msg;
    }

    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}