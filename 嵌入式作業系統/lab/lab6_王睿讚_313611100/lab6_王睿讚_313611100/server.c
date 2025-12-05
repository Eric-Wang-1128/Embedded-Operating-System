#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>    // for ftok, IPC_CREAT…
#include <sys/sem.h>    // for semget, semctl, IPC_RMID, SETVAL…

#define MAX_CLIENTS  10
#define MAX_BUFFER   256
#define EXPECTED_CLIENTS 4

int done_count = 0;
pthread_mutex_t count_mutex;
sem_t semaphore;
int account_balance = 0;

void *handle_client(void *arg) {
    int client_sock = *((int*)arg);
    free(arg);

    char buffer[MAX_BUFFER];
    char action[16];
    int amount;
    ssize_t n;

    // 持續接收，直到 client 關線
    while ((n = recv(client_sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[n] = '\0';
        // 只解析 action + amount
        if (sscanf(buffer, "%15s %d", action, &amount) != 2)
            continue;

        sem_wait(&semaphore);
        if (strcmp(action, "deposit") == 0) {
            account_balance += amount;
            printf("After deposit: %d\n", account_balance);
        } else if (strcmp(action, "withdraw") == 0) {
            if (account_balance >= amount) {
                account_balance -= amount;
            }
            printf("After withdraw: %d\n", account_balance);
        }
        fflush(stdout);
        sem_post(&semaphore);
        // 回一个简单 ACK 让 client 的 recv() 能拿到东西
        snprintf(buffer, sizeof(buffer), "ACK");
        send(client_sock, buffer, strlen(buffer), 0);
        // 釋放 CPU 給其他 thread
        sched_yield();
    }

    close(client_sock);
    // 這裡一個 client 處理完畢，更新計數
    pthread_mutex_lock(&count_mutex);
    done_count++;
    if (done_count == EXPECTED_CLIENTS) {
        // 全部 client 完成，清理 semaphore 並結束
        sem_destroy(&semaphore);
        printf("All %d clients done → semaphore destroyed, exiting.\n",
               EXPECTED_CLIENTS);
        fflush(stdout);
        exit(0);
    }
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    sem_init(&semaphore, 0, 1);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(atoi(argv[1]));

    int opt = 1;
    setsockopt(server_sock,
       	   SOL_SOCKET,
           SO_REUSEADDR,   // 让 bind() 可以复用处于 TIME_WAIT 的端口
           &opt,
           sizeof(opt));
   
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %s...\n", argv[1]);
    fflush(stdout);

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) continue;
        int *p = malloc(sizeof(int));
        *p = client_sock;
        pthread_create(&tid, NULL, handle_client, p);
        pthread_detach(tid);
    }

    // 永遠不會到這裡
    close(server_sock);
    sem_destroy(&semaphore);
    return 0;
}
