#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;
    char *id;
    int len;

    if (argc != 2) {
        printf("Usage: %s <student_id>\n", argv[0]);
        return 1;
    }

    id = argv[1];
    len = strlen(id);

    fd = open("/dev/lab3_dev", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    write(fd, id, len);
    close(fd);

    // 接著一秒一個字元送給顯示用裝置（讀的部分）
    fd = open("/dev/lab3_dev", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device for reading");
        return 1;
    }

    while (1) {
        char ch;
        read(fd, &ch, 1);
        printf("Display: %c\n", ch);  // 模擬七段顯示器輸出
        sleep(1);
    }

    close(fd);
    return 0;
}
