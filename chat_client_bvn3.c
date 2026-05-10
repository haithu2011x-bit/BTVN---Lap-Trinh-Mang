#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    fd_set fds;
    char buffer[1024];

    while (1) {
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        FD_SET(0, &fds);

        int maxfd = sock;
        select(maxfd + 1, &fds, NULL, NULL, NULL);

        if (FD_ISSET(0, &fds)) {
            fgets(buffer, sizeof(buffer), stdin);
            send(sock, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(sock, &fds)) {
            int r = recv(sock, buffer, sizeof(buffer), 0);
            if (r <= 0) break;
            buffer[r] = '\0';
            printf("%s", buffer);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}