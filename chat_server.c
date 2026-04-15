#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9000
#define MAX_CLIENTS 10

typedef struct {
    SOCKET socket;
    char name[50];
    int authenticated;
} Client;

void send_to_others(Client *clients, int sender_idx, char *message) {
    char buffer[512];
    time_t now = time(0);
    struct tm tstruct = *localtime(&now);
    char time_str[80];
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S", &tstruct);

    sprintf(buffer, "%s %s: %s", time_str, clients[sender_idx].name, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != INVALID_SOCKET && i != sender_idx && clients[i].authenticated) {
            send(clients[i].socket, buffer, strlen(buffer), 0);
        }
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(PORT) };
    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 3);

    Client clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = INVALID_SOCKET;
        clients[i].authenticated = 0;
    }

    printf("Chat Server dang chay tren port %d...\n", PORT);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        SOCKET max_sd = server_sock;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != INVALID_SOCKET) {
                FD_SET(clients[i].socket, &readfds);
                if (clients[i].socket > max_sd) max_sd = clients[i].socket;
            }
        }

        select(0, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_sock, &readfds)) {
            SOCKET new_sock = accept(server_sock, NULL, NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == INVALID_SOCKET) {
                    clients[i].socket = new_sock;
                    char *msg = "Vui long nhap ten theo cu phap 'client_id: client_name':\n";
                    send(new_sock, msg, strlen(msg), 0);
                    break;
                }
            }
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
                        // Kiem tra cu phap client_id: client_name
                        char id[20], name[50];
                        if (sscanf(buffer, "client_id: %s", name) == 1) {
                            strcpy(clients[i].name, name);
                            clients[i].authenticated = 1;
                            send(clients[i].socket, "Dang nhap thanh cong!\n", 22, 0);
                        } else {
                            send(clients[i].socket, "Sai cu phap! Thu lai:\n", 22, 0);
                        }
                    } else {
                        send_to_others(clients, i, buffer);
                    }
                }
            }
        }
    }
    return 0;
}