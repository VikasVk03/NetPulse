#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "9001"
#define BACKLOG 10
#define BUFFER_SIZE 1024

int main()
{
    struct addrinfo hints{}, *res, *p;
    int sockfd, new_fd;

    // Setup hints
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get address info
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    // Loop through results and bind
    for (p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        std::cerr << "Failed to bind\n";
        return 2;
    }

    freeaddrinfo(res);

    // Listen
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        return 1;
    }

    std::cout << "Server waiting on port " << PORT << "...\n";

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;

    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1)
    {
        perror("accept");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (true)
    {
        int bytes = recv(new_fd, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0)
            break;

        // Echo back
        send(new_fd, buffer, bytes, 0);
    }

    close(new_fd);
    close(sockfd);

    return 0;
}