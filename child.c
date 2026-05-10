#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

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

int main(int argc, char *argv[]) {
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);

    int sd = atoi(argv[1]);
    char buf[256];

    int r = recv(sd, buf, 255, 0);
    if (r <= 0) return 0;

    buf[r]=0;
    buf[strcspn(buf,"\r\n")] = 0;

    char cmd[64], fmt[64];
    int k = sscanf(buf, "%s %s", cmd, fmt);

    if (k != 2 || strcmp(cmd, "GET_TIME") != 0) {
        send(sd, "INVALID COMMAND\n", 16, 0);
        return 0;
    }

    int type = valid_format(fmt);
    if (!type) {
        send(sd, "INVALID FORMAT\n", 15, 0);
        return 0;
    }

    char out[64];
    format_time(out, type);
    strcat(out, "\n");
    send(sd, out, strlen(out), 0);

    return 0;
}