#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define MAX_CLIENT 100

int check_login(const char *user, const char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;
    char u[64], p[64];
    while (fscanf(f, "%s %s", u, p) != EOF) {
        if (strcmp(u, user) == 0 && strcmp(p, pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void run_command(const char *cmd) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s > out.txt", cmd);
    system(buf);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET client_socks[MAX_CLIENT] = {0};
    int authenticated[MAX_CLIENT] = {0};

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        SOCKET maxfd = server_fd;

        for (int i = 0; i < MAX_CLIENT; i++) {
            if (client_socks[i] > 0) {
                FD_SET(client_socks[i], &readfds);
                if (client_socks[i] > maxfd)
                    maxfd = client_socks[i];
            }
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            SOCKET c = accept(server_fd, NULL, NULL);
            for (int i = 0; i < MAX_CLIENT; i++) {
                if (client_socks[i] == 0) {
                    client_socks[i] = c;
                    authenticated[i] = 0;
                    send(c, "User: ", 6, 0);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            SOCKET sd = client_socks[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                char buf[1024];
                int r = recv(sd, buf, sizeof(buf) - 1, 0);
                if (r <= 0) {
                    closesocket(sd);
                    client_socks[i] = 0;
                    authenticated[i] = 0;
                    continue;
                }

                buf[r] = 0;
                buf[strcspn(buf, "\r\n")] = 0;

                if (!authenticated[i]) {
                    static char user[100];
                    static int stage = 0;

                    if (stage == 0) {
                        strcpy(user, buf);
                        send(sd, "Pass: ", 6, 0);
                        stage = 1;
                    } else {
                        if (check_login(user, buf)) {
                            send(sd, "Login OK\n$ ", 11, 0);
                            authenticated[i] = 1;
                        } else {
                            send(sd, "Login FAILED\n", 13, 0);
                            closesocket(sd);
                            client_socks[i] = 0;
                        }
                        stage = 0;
                    }
                    continue;
                }

                run_command(buf);

                FILE *f = fopen("out.txt", "r");
                if (!f) continue;

                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    send(sd, line, strlen(line), 0);
                }
                fclose(f);

                send(sd, "\n$ ", 3, 0);
            }
        }
    }

    WSACleanup();
    return 0;
}