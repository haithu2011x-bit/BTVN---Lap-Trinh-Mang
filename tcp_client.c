#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: tcp_client <IP> <PORT>\n");
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server.sin_addr);

    if (connect(client, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connect failed!\n");
        return 1;
    }

    char buffer[1024];
    while (1) {
        printf("Input: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(client, buffer, strlen(buffer), 0);
    }

    closesocket(client);
    WSACleanup();
    return 0;
}