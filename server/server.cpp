#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocol.h"

#define PORT 9000

int main()
{
    int server_fd, client_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket filed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        return 1;
    }

    listen(server_fd, 5);
    std::cout << "Server listening on port " << PORT << std::endl;

    client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_sock < 0)
    {
        perror("accept");
        return 1;
    }

    std::cout << "Client connected\n";

    while (true)
    {
        Packet pkt;
        int received = 0;

        while (received < sizeof(Packet))
        {
            int bytes = recv(client_sock, ((char *)&pkt) + received,
                             sizeof(Packet) - received, 0);

            if (bytes <= 0)
            {
                std::cout << "Client disconnected\n";
                return 0;
            }

            received += bytes;
        }

        if (pkt.type == 1)
        {
            // latency -> echo back
            int sent = 0;
            while (sent < sizeof(pkt))
            {
                int bytes = send(client_sock, ((char *)&pkt) + sent, sizeof(pkt) - sent, 0);

                if (bytes <= 0)
                    break;
                sent += bytes;
            }
        }
        else if (pkt.type == 2)
        {
            // bandwith -> do noting
        }

        // echo back full packet
        // send(client_sock, &pkt, sizeof(pkt), 0);

        int sent = 0;
        while (sent < sizeof(pkt))
        {
            int bytes = send(client_sock,
                             ((char *)&pkt) + sent,
                             sizeof(pkt) - sent, 0);
            if (bytes <= 0)
                break;
            sent += bytes;
        }
    }

    close(client_sock);
    close(server_fd);

    return 0;
}