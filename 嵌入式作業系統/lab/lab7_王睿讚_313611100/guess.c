#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

typedef struct {
    int        guess;
    char       result[8];
    pid_t      game_pid;
    pid_t      guess_pid;
} data_t;

static data_t *data;
static volatile int ready = 0;

void sigusr1_handler(int sig) {
    ready = 1;
    printf("[guess] 收到回覆：%s\n", data->result);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <key> <upper_bound> <game_pid>\n", argv[0]);
        exit(1);
    }

    key_t   key      = atoi(argv[1]);
    int     upper    = atoi(argv[2]);
    pid_t   game_pid = atoi(argv[3]);

    int shmid = shmget(key, sizeof(data_t), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    data = (data_t *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    data->guess_pid = getpid();  // 写入自己的 PID
    signal(SIGUSR1, sigusr1_handler);

    int low = 1, high = upper;
    while (low <= high) {
        int mid = (low + high) / 2;
        printf("[guess] Guessing: %d\n", mid);
        data->guess = mid;
        ready = 0;

        kill(game_pid, SIGUSR1);               // 通知 game
        while (!ready) usleep(1000);           // 等 game 回信

        if (strcmp(data->result, "correct") == 0) {
            printf("[guess] Got it! The number is %d\n", mid);
            break;
        } else if (strcmp(data->result, "larger") == 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }

        usleep(300000);  // 每次延迟 0.3 秒，更容易录影演示
    }

    shmdt(data);  // detach
    return 0;
}
