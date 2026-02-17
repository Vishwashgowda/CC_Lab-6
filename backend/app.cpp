#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {

    // Get container hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname failed");
        return 1;
    }
    hostname[255] = '\0';

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Setup server address
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    // Start listening
    if (listen(server_fd, 10) == -1) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Backend server running on port 8080\n";
    std::cout << "Hostname: " << hostname << std::endl;

    // Main server loop
    while (true) {

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Accept client connection
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }

        std::cout << "Connection received from client" << std::endl;

        // Build HTTP response
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Served by backend: " + std::string(hostname) + "\n";

        // Send response
        ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);

        if (bytes_sent == -1) {
            perror("send failed");
        }

        // Close client connection
        close(client_fd);
    }

    // Close server socket (never reached normally)
    close(server_fd);

    return 0;
}
