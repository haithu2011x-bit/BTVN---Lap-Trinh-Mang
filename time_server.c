#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define MAX_CLIENT 100

int valid_format(const char *fmt) {
    if (!strcmp(fmt, "dd/mm/yyyy")) return 1;
    if (!strcmp(fmt, "dd/mm/yy")) return 2;
    if (!strcmp(fmt, "mm/dd/yyyy")) return 3;
    if (!strcmp(fmt, "mm/dd/yy")) return 4;
    return 0;
}

void format_time(char *out, int type) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (type == 1) sprintf(out, "%02d/%02d/%04d", t->tm_mday, t->tm_mon+1, t->tm_year+1900);
    if (type == 2) sprintf(out, "%02d/%02d/%02d", t->tm_mday, t->tm_mon+1, (t->tm_year+1900)%100);
    if (type == 3) sprintf(out, "%02d/%02d/%04d", t->tm_mon+1, t->tm_mday, t->tm_year+1900);
    if (type == 4) sprintf(out, "%02d/%02d/%02d", t->tm_mon+1, t->tm_mday, (t->tm_year+1900)%100);
}

void run_child(SOCKET client) {
    char buf[256];
    int n = recv(client, buf, sizeof(buf)-1, 0);
    if (n <= 0) exit(0);

    buf[n] = 0;
    buf[strcspn(buf, "\r\n")] = 0;

    char cmd[64], fmt[64];
    int k = sscanf(buf, "%s %s", cmd, fmt);

    if (k != 2 || strcmp(cmd, "GET_TIME") != 0) {
        send(client, "INVALID COMMAND\n", 16, 0);
        exit(0);
    }

    int f = valid_format(fmt);
    if (!f) {
        send(client, "INVALID FORMAT\n", 15, 0);
        exit(0);
    }

    char tbuf[64];
    format_time(tbuf, f);
    strcat(tbuf, "\n");
    send(client, tbuf, strlen(tbuf), 0);

    exit(0);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET clients[MAX_CLIENT] = {0};

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        SOCKET maxfd = server_fd;

        for (int i = 0; i < MAX_CLIENT; i++) {
            if (clients[i] > 0) {
                FD_SET(clients[i], &readfds);
                if (clients[i] > maxfd) maxfd = clients[i];
            }
        }

        select(maxfd+1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            SOCKET c = accept(server_fd, NULL, NULL);
            for (int i = 0; i < MAX_CLIENT; i++) {
                if (clients[i] == 0) {
                    clients[i] = c;
                    break;
                }
            }
            send(c, "OK\n", 3, 0);
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            SOCKET sd = clients[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                STARTUPINFOA si = {0};
                PROCESS_INFORMATION pi = {0};
                si.cb = sizeof(si);

                char cmd[128];
                sprintf(cmd, "child.exe %d", sd);

                STARTUPINFOA si2 = {0};
                si2.cb = sizeof(si2);

                if (CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi)) {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }

                closesocket(sd);
                clients[i] = 0;
            }
        }
    }
    WSACleanup();
    return 0;
}