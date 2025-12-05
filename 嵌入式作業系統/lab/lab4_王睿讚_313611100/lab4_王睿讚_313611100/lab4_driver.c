// ========== mydev.c (Lab4 Driver) ==========
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "mydev"
#define BUF_SIZE 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Wang");
MODULE_DESCRIPTION("Lab4 - Name to 16-segment encoding driver");
MODULE_VERSION("1.0");

static char name_buffer[BUF_SIZE];
static int name_len = 0;
static int name_index = 0;

static struct class *dev_class;
static struct cdev my_cdev;
static dev_t dev_num;

// 每個字元對應的 16 段顯示器 bit pattern
static uint16_t seg_for_c[27] = {
    0b1111001100010001, // A
    0b0000011100000101, // b
    0b1100111100000000, // C
    0b0000011001000101, // d
    0b1000011100000001, // E
    0b1000001100000001, // F
    0b1001111100010000, // G
    0b0011001100010001, // H
    0b1100110001000100, // I
    0b1100010001000100, // J
    0b0000000001101100, // K
    0b0000111100000000, // L
    0b0011001110100000, // M
    0b0011001110001000, // N
    0b1111111100000000, // O
    0b1000001101000001, // P
    0b0111000001010000, //q
    0b1110001100011001, //R
    0b1101110100010001, //S
    0b1100000001000100, //T
    0b0011111100000000, //U
    0b0000001100100010, //V
    0b0011001100001010, //W
    0b0000000010101010, //X
    0b0000000010100100, //Y
    0b1100110000100010, //Z
    0b0000000000000000, //default
};

static ssize_t mydev_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    if (len >= BUF_SIZE) len = BUF_SIZE - 1;
    if (copy_from_user(name_buffer, buf, len)) return -EFAULT;

    name_buffer[len] = '\0';
    name_len = len;
    name_index = 0;

    pr_info("[mydev] name stored: %s\n", name_buffer);
    return len;
}

static ssize_t mydev_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    uint16_t encoded;
    char ch;
    uint8_t pattern[16];

    if (name_len == 0) return 0;
    ch = name_buffer[name_index];

    if (ch >= 'A' && ch <= 'Z')
        encoded = seg_for_c[ch - 'A'];
    else if (ch >= 'a' && ch <= 'z')
        encoded = seg_for_c[ch - 'a'];
    else
        encoded = seg_for_c[26]; // default

    for (int i = 0; i < 16; i++) {
        pattern[15-i] = (encoded >> i) & 1;
    }

    if (copy_to_user(buf, pattern, sizeof(pattern))) return -EFAULT;

    name_index = (name_index + 1) % name_len;
    return sizeof(pattern);
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = mydev_read,
    .write = mydev_write
};

static int __init mydev_init(void) {
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_num, 1);

    dev_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);

    pr_info("[mydev] driver loaded\n");
    return 0;
}

static void __exit mydev_exit(void) {
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("[mydev] driver unloaded\n");
}

module_init(mydev_init);
module_exit(mydev_exit);
