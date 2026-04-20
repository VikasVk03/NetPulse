#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <chrono>

#define PORT "9001"
#define NUM_PACKETS 100
#define BUFFER_SIZE 1024

using namespace std;
using namespace std::chrono;

int main()
{
    struct addrinfo hints{}, *res, *p;
    int sockfd;

    // Setup hints
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get address info (localhost)
    if (getaddrinfo("127.0.0.1", PORT, &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    // Connect
    for (p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        cerr << "Failed to connect\n";
        return 2;
    }

    freeaddrinfo(res);

    char buffer[BUFFER_SIZE] = "ping";
    char recv_buffer[BUFFER_SIZE];

    long long total_latency = 0;
    long long max_latency = 0;

    for (int i = 0; i < NUM_PACKETS; i++)
    {
        auto start = high_resolution_clock::now();

        send(sockfd, buffer, strlen(buffer), 0);
        recv(sockfd, recv_buffer, BUFFER_SIZE, 0);

        auto end = high_resolution_clock::now();

        auto latency = duration_cast<microseconds>(end - start).count();

        total_latency += latency;
        if (latency > max_latency)
            max_latency = latency;
    }

    cout << "\n==== Latency Stats ====\n";
    cout << "Packets Sent: " << NUM_PACKETS << endl;
    cout << "Avg RTT: " << total_latency / NUM_PACKETS << " microseconds\n";
    cout << "Max RTT: " << max_latency << " microseconds\n";

    close(sockfd);

    return 0;
}