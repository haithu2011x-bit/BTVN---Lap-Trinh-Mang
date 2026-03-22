#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: tcp_server <PORT> <welcome_file> <output_file>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    char *welcome_file = argv[2];
    char *output_file = argv[3];

    // Đọc câu chào
    FILE *fw = fopen(welcome_file, "r");
    if (!fw) {
        printf("Cannot open welcome file!\n");
        return 1;
    }
    char welcome[1024];
    fgets(welcome, sizeof(welcome), fw);
    fclose(fw);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 5);

    printf("Server listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        SOCKET client = accept(server, (struct sockaddr*)&clientAddr, &len);

        // gửi câu chào
        send(client, welcome, strlen(welcome), 0);

        // ghi dữ liệu client vào file
        FILE *fo = fopen(output_file, "a");
        char buffer[1024];
        int bytes;

        while ((bytes = recv(client, buffer, sizeof(buffer)-1, 0)) > 0) {
            buffer[bytes] = 0;
            fprintf(fo, "%s", buffer);
            fflush(fo);
        }

        fclose(fo);
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}