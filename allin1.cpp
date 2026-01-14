#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <fstream>  // REQUIRED: To read files
#include <string>

const int PORT = 8080;
const int BUFFER_SIZE = 4096; // Increased buffer for larger requests
const int THREAD_POOL_SIZE = 4;
// =================================================================
// 1. THE THREAD POOL CLASS
// =================================================================
class ThreadPool {
private:
    std::vector<std::thread> workers;              // The permanent workers
    std::queue<std::function<void()>> tasks;       // The queue of jobs (tokens)
    
    std::mutex queue_mutex;                        // Lock to protect the queue
    std::condition_variable condition;             // To wake up workers
    bool stop;                                     // Flag to stop the pool

public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i) {
            // Create 'threads' number of workers
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;

                    // The "Critical Section" - Accessing the shared queue
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        
                        // Wait until there is a task OR we are stopping
                        this->condition.wait(lock, [this]{ 
                            return this->stop || !this->tasks.empty(); 
                        });

                        if(this->stop && this->tasks.empty())
                            return;

                        // Get the task from the queue
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // Execute the task (Talk to the client)
                    task();
                }
            });
        }
    }

    // Function to add new work to the queue
    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(task);
        }
        condition.notify_one(); // Wake up one worker!
    }

    // Destructor to clean up threads
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker : workers)
            worker.join();
    }
};
std::string get_mime_type(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css")  != std::string::npos) return "text/css";
    if (path.find(".js")   != std::string::npos) return "application/javascript";
    if (path.find(".png")  != std::string::npos) return "image/png";
    if (path.find(".jpg")  != std::string::npos) return "image/jpeg";
    if (path.find(".gif")  != std::string::npos) return "image/gif";
    return "text/plain"; // Default
}

// 2. Updated File Loader (Binary Mode)
// We use std::ios::binary to prevent corrupting images
std::string load_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary); 
    if (!file.is_open()) return "";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 3. Updated Client Handler
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE - 1);

    // Parse the Request
    std::istringstream request_stream(buffer);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    // Default to index.html
    if (path == "/") path = "/index.html";
    
    std::string filepath = "www" + path; 
    std::string content = load_file(filepath);
    std::string response;

    if (!content.empty()) {
        // SUCCESS: 200 OK
        std::string mime_type = get_mime_type(filepath);
        
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: " + mime_type + "\r\n"
                   "Content-Length: " + std::to_string(content.size()) + "\r\n"
                   "\r\n" + 
                   content;
    } else {
        // ERROR: 404 Not Found
        std::string error_msg = "<html><body><h1>404 Not Found</h1></body></html>";
        response = "HTTP/1.1 404 Not Found\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: " + std::to_string(error_msg.size()) + "\r\n"
                   "\r\n" + 
                   error_msg;
    }

    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}
// =================================================================
// 3. MAIN SERVER LOOP
// =================================================================
int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Create Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) { perror("socket failed"); exit(EXIT_FAILURE); }

    // Socket Options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt"); exit(EXIT_FAILURE);
    }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed"); exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    // Initialize Thread Pool with 4 threads
    ThreadPool pool(THREAD_POOL_SIZE);

    std::cout << "Server (Thread Pool) listening on port " << PORT << "...\n";

    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        
        int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        // Instead of handling it here, we throw it into the pool!
        pool.enqueue([new_socket] {
            handle_client(new_socket);
        });
    }

    return 0;
}