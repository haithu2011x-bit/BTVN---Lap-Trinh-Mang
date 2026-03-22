#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: sv_server <PORT> <log_file>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    char *log_file = argv[2];

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 5);

    printf("sv_server listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        SOCKET client = accept(server, (struct sockaddr*)&clientAddr, &len);

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));

        char buffer[1024];
        int bytes = recv(client, buffer, sizeof(buffer)-1, 0);
        buffer[bytes] = 0;

        // lấy thời gian hiện tại
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", t);

        // ghi file log
        FILE *f = fopen(log_file, "a");
        fprintf(f, "%s %s %s\n", ipStr, timeStr, buffer);
        fclose(f);

        printf("Received: %s %s %s\n", ipStr, timeStr, buffer);

        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}