#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: sv_client <IP> <PORT>\n");
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

    char mssv[50], name[100], dob[50], gpa[10];
    printf("MSSV: "); fgets(mssv, sizeof(mssv), stdin);
    printf("Ho ten: "); fgets(name, sizeof(name), stdin);
    printf("Ngay sinh (YYYY-MM-DD): "); fgets(dob, sizeof(dob), stdin);
    printf("Diem trung binh: "); fgets(gpa, sizeof(gpa), stdin);

    char data[512];
    snprintf(data, sizeof(data), "%s %s %s %s",
             strtok(mssv, "\n"), strtok(name, "\n"),
             strtok(dob, "\n"), strtok(gpa, "\n"));

    send(client, data, strlen(data), 0);

    closesocket(client);
    WSACleanup();
    return 0;
}