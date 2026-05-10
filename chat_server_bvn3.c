#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define MAX_CLIENT 100

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    SOCKET client_socks[MAX_CLIENT];
    char client_ids[MAX_CLIENT][50];

    fd_set readfds;

    for (int i = 0; i < MAX_CLIENT; i++) {
        client_socks[i] = 0;
        client_ids[i][0] = '\0';
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        SOCKET max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENT; i++) {
            SOCKET sd = client_socks[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_fd) max_fd = sd;
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
            char msg[] = "Nhap ten theo cu phap: client_id: name\n";
            send(new_socket, msg, strlen(msg), 0);

            for (int i = 0; i < MAX_CLIENT; i++) {
                if (client_socks[i] == 0) {
                    client_socks[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            SOCKET sd = client_socks[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                char buffer[1024];
                int valread = recv(sd, buffer, sizeof(buffer), 0);

                if (valread <= 0) {
                    closesocket(sd);
                    client_socks[i] = 0;
                    client_ids[i][0] = '\0';
                    continue;
                }

                buffer[valread] = '\0';

                if (client_ids[i][0] == '\0') {
                    char *p = strchr(buffer, ':');
                    if (!p) {
                        char err[] = "Sai cu phap. Vui long nhap: client_id: name\n";
                        send(sd, err, strlen(err), 0);
                        continue;
                    }

                    *p = '\0';
                    strcpy(client_ids[i], buffer);

                    char ok[] = "Dang ky thanh cong!\n";
                    send(sd, ok, strlen(ok), 0);
                    continue;
                }

                char out[1200];
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                char ts[64];
                strftime(ts, sizeof(ts), "%Y/%m/%d %I:%M:%S%p", t);

                snprintf(out, sizeof(out), "%s %s: %s", ts, client_ids[i], buffer);

                for (int j = 0; j < MAX_CLIENT; j++) {
                    if (client_socks[j] > 0 && j != i)
                        send(client_socks[j], out, strlen(out), 0);
                }
            }
        }
    }

    WSACleanup();
    return 0;
}