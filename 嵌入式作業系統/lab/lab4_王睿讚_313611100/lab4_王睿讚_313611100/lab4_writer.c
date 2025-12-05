// ========== writer.c (Lab4) ==========
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <your_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *dev_path = "/dev/mydev";
    const char *name = argv[1];
    int len = strlen(name);

    // 傳送名字到 driver
    int fd = open(dev_path, O_WRONLY);
    if (fd < 0) {
        perror("open(write)");
        return 1;
    }
    write(fd, name, len);
    close(fd);

    // 再次開啟作為讀取
    fd = open(dev_path, O_RDONLY);
    if (fd < 0) {
        perror("open(read)");
        return 1;
    }

    close(fd);
    return 0;
}
