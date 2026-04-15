#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9001
#define MAX_CLIENTS 5

typedef struct {
    SOCKET socket;
    int authenticated;
} TelnetClient;

int check_login(char *input) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;
    char line[100];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, input) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(PORT) };
    
    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 3);

    TelnetClient clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].socket = INVALID_SOCKET;

    printf("Telnet Server dang chay tren port %d...\n", PORT);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != INVALID_SOCKET) FD_SET(clients[i].socket, &readfds);
        }

        select(0, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_sock, &readfds)) {
            SOCKET new_sock = accept(server_sock, NULL, NULL);
            int accepted = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == INVALID_SOCKET) {
                    clients[i].socket = new_sock;
                    clients[i].authenticated = 0;
                    send(new_sock, "Nhap user pass (vd: admin admin):\n", 35, 0);
                    accepted = 1;
                    break;
                }
            }
            if (!accepted) closesocket(new_sock);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != INVALID_SOCKET && FD_ISSET(clients[i].socket, &readfds)) {
                char buffer[1024] = {0};
                int valread = recv(clients[i].socket, buffer, 1024, 0);
                
                if (valread <= 0) {
                    closesocket(clients[i].socket);
                    clients[i].socket = INVALID_SOCKET;
                } else {
                    buffer[strcspn(buffer, "\r\n")] = 0;

                    if (!clients[i].authenticated) {
                        if (check_login(buffer)) {
                            clients[i].authenticated = 1;
                            send(clients[i].socket, "Dang nhap thanh cong! Nhap lenh:\n", 33, 0);
                        } else {
                            send(clients[i].socket, "Loi dang nhap! Thu lai:\n", 24, 0);
                        }
                    } else {
                        char cmd[1100];
                        sprintf(cmd, "%s > out.txt", buffer);
                        system(cmd);

                        FILE *f = fopen("out.txt", "rb");
                        if (f) {
                            char file_buf[1024];
                            int n;
                            while ((n = fread(file_buf, 1, sizeof(file_buf), f)) > 0) {
                                send(clients[i].socket, file_buf, n, 0);
                            }
                            fclose(f);
                        }
                    }
                }
            }
        }
    }
    WSACleanup();
    return 0;
}