#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER 256

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <deposit/withdraw> <amount> <times>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int   port      = atoi(argv[2]);
    char *action    = argv[3];
    int   amount    = atoi(argv[4]);
    int   times     = atoi(argv[5]);

    if (strcmp(action, "deposit") != 0 &&
        strcmp(action, "withdraw") != 0) {
        fprintf(stderr, "action must be deposit or withdraw\n");
        exit(1);
    }
    if (times > 500) times = 500;     // 上限 500 次
    if (amount <= 0) {
        fprintf(stderr, "amount must be positive\n");
        exit(1);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    inet_pton(AF_INET, server_ip, &srv.sin_addr);

    if (connect(sock, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("connect");
        close(sock);
        exit(1);
    }

    char buffer[MAX_BUFFER];
    for (int i = 0; i < times; i++) {
        // 每次送一次 "action amount"
        snprintf(buffer, sizeof(buffer), "%s %d", action, amount);
        send(sock, buffer, strlen(buffer), 0);

        // 等伺服器回個 ack（這裡不顯示出來）
        recv(sock, buffer, sizeof(buffer)-1, 0);

        // 1ms 暫停，讓其他 client 有機會獲得 CPU
        usleep(1000);
    }

    close(sock);
    return 0;
}
