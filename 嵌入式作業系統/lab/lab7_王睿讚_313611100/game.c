#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct {
    int        guess;
    char       result[8];
    pid_t      game_pid;
    pid_t      guess_pid;
} data_t;

data_t *data;
int target;
static int shmid;
static volatile sig_atomic_t guessed = 0;

void sigusr1_handler(int sig) {
    if (data->guess == target) {
        strcpy(data->result, "correct");
        guessed = 1;
    } else if (data->guess < target) {
        strcpy(data->result, "larger");
    } else {
        strcpy(data->result, "smaller");
    }
    printf("[game] 收到猜測 %d，回覆：%s\n", data->guess, data->result);
    fflush(stdout);
    kill(data->guess_pid, SIGUSR1);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key> <target_number>\n", argv[0]);
        exit(1);
    }

    key_t key   = atoi(argv[1]);      // ← 直接用命令行传入的整数 key
    target      = atoi(argv[2]);

    int shmid = shmget(key, sizeof(data_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    data = (data_t *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    

    data->game_pid = getpid();
    printf("[game] Game PID: %d\n", data->game_pid);
    signal(SIGUSR1, sigusr1_handler);
    
    while (!guessed) {
        pause();  // 等待 guess 发信号过来
    }
    printf("[game] 猜中 %d，自动清理 shared-memory\n", target);

    // detach & remove
    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
