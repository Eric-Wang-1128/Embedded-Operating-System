
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_PENDING 5

int sockfd; // global for signal handler to close

void sigchld_handler(int signum) {
    // 處理 zombie process
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void sigint_handler(int signum) {
    // Ctrl+C 時釋放 socket
    close(sockfd);
    printf("Server socket closed.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    signal(SIGCHLD, sigchld_handler); // zombie handler
    signal(SIGINT, sigint_handler);   // Ctrl+C handler

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }

    // 避免 address already in use
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        exit(1);
    }

    if (listen(sockfd, MAX_PENDING) < 0) {
        perror("listen error");
        exit(1);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept error");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {  // child process
            close(sockfd); // 子行程不需要用 server socket

            dup2(client_fd, STDOUT_FILENO);
            dup2(client_fd, STDERR_FILENO); // 避免 sl 輸出錯誤訊息斷線
            close(client_fd);
            execlp("/home/ericwang/sl-5.02/sl", "sl", "-l", NULL); // fire up train!
            perror("exec error");
            exit(1);
        } else {
            close(client_fd); // parent 不需要 client socket
            printf("Train ID %d\n", pid);
        }
    }

    return 0;
}
