/* hw2.c - 多人連線版外送平台 Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define MAX_CLIENTS  FD_SETSIZE
#define BUF_SIZE     256
#define HW2_DEV      "/dev/HW2_dev"

/* Menu definitions */
typedef struct {
    const char *name;
    int price;
} menu_item_t;

typedef struct {
    const char *shop_name;
    int distance;
    menu_item_t items[10];
    int item_count;
} shop_t;

shop_t shops[] = {
    {"Dessert shop", 3, {{"cookie",60},{"cake",80}}, 2},
    {"Beverage shop",5, {{"tea",40},{"boba",70}},     2},
    {"Diner",       8, {{"fried-rice",120},{"Egg-drop-soup",50}}, 2},
};
int shop_count = sizeof(shops)/sizeof(shops[0]);

/* Per-client state */
typedef struct {
    int fd;
    int order_made;
    int shop_idx;
    int total_price;
} client_t;

/* Send the shop list and menus */
void send_shop_list(int fd) {
    char buf[BUF_SIZE];
    int len;
    for (int i = 0; i < shop_count; i++) {
        len = snprintf(buf, sizeof(buf), "%s:%dkm\n", shops[i].shop_name, shops[i].distance);
        send(fd, buf, len, 0);
        /* list items */
        send(fd, "- ", 2, 0);
        for (int j = 0; j < shops[i].item_count; j++) {
            len = snprintf(buf, sizeof(buf), "%s:$%d",
                           shops[i].items[j].name,
                           shops[i].items[j].price);
            send(fd, buf, len, 0);
            if (j < shops[i].item_count - 1) send(fd, "|", 1, 0);
        }
        send(fd, "\n", 1, 0);
    }
}

/* Display on hw2 device */
void hw2_display(int distance, int price) {
    int fd = open(HW2_DEV, O_WRONLY);
    if (fd < 0) { perror("open hw2"); return; }
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d,%d", distance, price);
    write(fd, buf, n);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int listen_fd, conn_fd, maxfd, nready;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    fd_set allset, rset;
    client_t clients[MAX_CLIENTS];
    char buf[BUF_SIZE];

    /* 初始化 clients */
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].fd = -1;

    /* 建立 listening socket */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listen_fd, 10);

    FD_ZERO(&allset);
    FD_SET(listen_fd, &allset);
    maxfd = listen_fd;

    while (1) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        /* 新連線 */
        if (FD_ISSET(listen_fd, &rset)) {
            clilen = sizeof(cliaddr);
            conn_fd = accept(listen_fd, (struct sockaddr*)&cliaddr, &clilen);
            /* 加入 clients */
            int i;
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd < 0) {
                    clients[i].fd = conn_fd;
                    clients[i].order_made = 0;
                    clients[i].total_price = 0;
                    break;
                }
            }
            if (i == MAX_CLIENTS) { close(conn_fd); }
            FD_SET(conn_fd, &allset);
            if (conn_fd > maxfd) maxfd = conn_fd;
            if (--nready <= 0) continue;
        }
        /* 處理各 client 指令 */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sockfd = clients[i].fd;
            if (sockfd < 0) continue;
            if (FD_ISSET(sockfd, &rset)) {
                int n = read(sockfd, buf, BUF_SIZE-1);
                if (n <= 0) {
                    /* client 關閉 */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    clients[i].fd = -1;
                } else {
                    buf[n] = '\0';
                    /* 去除換行 */
                    if (buf[n-1] == '\n') buf[n-1] = '\0';
                    if (strcmp(buf, "shop list") == 0) {
                        send_shop_list(sockfd);
                    } else if (strncmp(buf, "order ", 6) == 0) {
                        char item[32]; int qty;
                        if (sscanf(buf+6, "%31s %d", item, &qty) == 2) {
                            int found = 0;
                            for (int s = 0; s < shop_count; s++) {
                                for (int j = 0; j < shops[s].item_count; j++) {
                                    if (strcmp(item, shops[s].items[j].name)==0) {
                                        if (!clients[i].order_made) {
                                            clients[i].shop_idx = s;
                                            clients[i].order_made = 1;
                                        }
                                        if (clients[i].shop_idx == s) {
                                            clients[i].total_price += shops[s].items[j].price * qty;
                                        }
                                        /* 回傳目前已點 */
                                        snprintf(buf, sizeof(buf), "%s %d\n", item, qty);
                                        send(sockfd, buf, strlen(buf), 0);
                                        found = 1;
                                    }
                                }
                            }
                            if (!found) send(sockfd, "Item not found\n", 15, 0);
                        } else {
                            send(sockfd, "Order format error\n", 20, 0);
                        }
                    } else if (strcmp(buf, "confirm") == 0) {
                        if (!clients[i].order_made) {
                            send(sockfd, "Please order some meals\n", 27, 0);
                        } else {
                            send(sockfd, "Please wait a few minutes...\n", 29, 0);
                            hw2_display(shops[clients[i].shop_idx].distance,
                                         clients[i].total_price);
                            snprintf(buf, sizeof(buf),
                                     "Delivery has arrived and you need to pay %d$\n",
                                     clients[i].total_price);
                            send(sockfd, buf, strlen(buf), 0);
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                            clients[i].fd = -1;
                        }
                    } else if (strcmp(buf, "cancel") == 0) {
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        clients[i].fd = -1;
                    } else {
                        send(sockfd, "Unknown command\n", 16, 0);
                    }
                }
                if (--nready <= 0) break;
            }
        }
    }
    return 0;
}
