#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocol.h"
#include "timer.h"

#define PORT 9000
#define NUM_PACKETS 1000

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "Connection Failed\n";
        return 1;
    }

    std::vector<double> latencies;

    // ---------------- LATENCY TEST ----------------
    for (int i = 0; i < NUM_PACKETS; i++)
    {
        Packet pkt;
        pkt.type = 1;
        pkt.seq = i;
        pkt.timestamp = now_us();

        // send(sock, &pkt, sizeof(pkt), 0);

        int sent = 0;
        while (sent < sizeof(pkt))
        {
            int bytes = send(sock, ((char *)&pkt) + sent,
                             sizeof(pkt) - sent, 0);
            if (bytes <= 0)
                break;
            sent += bytes;
        }

        // recv(sock, &pkt, sizeof(pkt), 0);

        int received = 0;
        while (received < sizeof(pkt))
        {
            int bytes = recv(sock, ((char *)&pkt) + received,
                             sizeof(pkt) - received, 0);
            if (bytes <= 0)
                break;
            received += bytes;
        }

        uint64_t rtt = now_us() - pkt.timestamp;
        latencies.push_back(rtt / 1000.0); // ms
    }

    // ---------------- METRICS ----------------
    std::sort(latencies.begin(), latencies.end());

    double sum = 0;
    for (auto l : latencies)
        sum += l;

    double avg = sum / latencies.size();
    double min = latencies.front();
    double max = latencies.back();
    double p95 = latencies[latencies.size() * 0.95];
    double p99 = latencies[latencies.size() * 0.99];

    // ---------------- BANDWIDTH TEST ----------------
    const int TEST_DURATION = 5; // seconds
    char buffer[PAYLOAD_SIZE] = {0};

    int total_bytes = 0;
    uint64_t start = now_us();

    Packet pkt;
    pkt.type = 2;

    while ((now_us() - start) < TEST_DURATION * 1e6)
    {
        int sent = 0;
        while (sent < sizeof(pkt))
        {
            int bytes = send(sock, ((char *)&pkt) + sent,
                             sizeof(pkt) - sent, 0);
            if (bytes <= 0)
                break;
            sent += bytes;
        }

        // send(sock, ((char*)&pkt), sizeof(pkt), 0);
        total_bytes += sizeof(pkt);
    }

    double duration = (now_us() - start) / 1e6;
    double bandwidth = (total_bytes * 8.0) / (duration * 1e6); // Mbps

    // ---------------- OUTPUT ----------------
    std::cout << "\n--- Network Analyzer Report ---\n";

    std::cout << "\nLatency (ms):\n";
    std::cout << "  avg: " << avg << "\n";
    std::cout << "  min: " << min << "\n";
    std::cout << "  max: " << max << "\n";
    std::cout << "  p95: " << p95 << "\n";
    std::cout << "  p99: " << p99 << "\n";

    std::cout << "\nBandwidth:\n";
    std::cout << "  " << bandwidth << " Mbps\n";

    close(sock);
    return 0;
}