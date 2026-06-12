#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

#define TARGET_IP "192.168.1.1"
#define START_PORT 1
#define END_PORT 1024
#define TIMEOUT_SEC 1

int scan_port(const char *ip, int port) {
    int sock;
    struct sockaddr_in addr;
    struct timeval timeout;
    fd_set fdset;
    int flags, res;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    res = select(sock + 1, NULL, &fdset, NULL, &timeout);
    if (res == 1) {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) {
            close(sock);
            return 1; // Port open
        }
    }
    close(sock);
    return 0;
}

int main() {
    printf("Scanning %s for open ports %d-%d\n", TARGET_IP, START_PORT, END_PORT);
    for (int port = START_PORT; port <= END_PORT; port++) {
        if (scan_port(TARGET_IP, port)) {
            printf("Port %d is OPEN\n", port);
        }
    }
    printf("Scan complete.\n");
    return 0;
}
