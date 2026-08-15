#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define GPIO_BASE   512
#define GPIO_COUNT  3

static int gpio_export(int gpio)
{
    int fd;
    char buf[16];
    int len;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("open export");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);

    if (write(fd, buf, len) < 0) {
        /* すでにexport済みならエラーにしない */
        if (errno != EBUSY) {
            perror("write export");
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

static int gpio_unexport(int gpio)
{
    int fd;
    char buf[16];
    int len;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);

    close(fd);
    return 0;
}

static int gpio_direction_out(int gpio)
{
    char path[64];
    int fd;

    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/direction", gpio);

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open direction");
        return -1;
    }

    if (write(fd, "out", 3) < 0) {
        perror("write direction");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int gpio_write(int gpio, int value)
{
    char path[64];
    int fd;

    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/value", gpio);

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open value");
        return -1;
    }

    if (value)
        write(fd, "1", 1);
    else
        write(fd, "0", 1);

    close(fd);
    return 0;
}

int main(void)
{
    int i;
    int gpio;

    printf("AXI GPIO test\n");

    /* GPIO 512～514 を初期化 */
    for (i = 0; i < GPIO_COUNT; i++) {
        gpio = GPIO_BASE + i;

        if (gpio_export(gpio) < 0)
            return 1;

        /* sysfsにgpioディレクトリが作られるまで少し待つ */
        usleep(100000);

        if (gpio_direction_out(gpio) < 0)
            return 1;

        gpio_write(gpio, 0);
    }

    while (1) {

        /* Port 0 → 1 → 2 を順番にON */
        for (i = 0; i < GPIO_COUNT; i++) {
            gpio = GPIO_BASE + i;

            printf("Port %d ON\n", i);
            gpio_write(gpio, 1);

            sleep(1);
        }

        /* Port 0 → 1 → 2 を順番にOFF */
        for (i = 0; i < GPIO_COUNT; i++) {
            gpio = GPIO_BASE + i;

            printf("Port %d OFF\n", i);
            gpio_write(gpio, 0);

            sleep(1);
        }
    }

    /* ここには通常到達しない */
    for (i = 0; i < GPIO_COUNT; i++)
        gpio_unexport(GPIO_BASE + i);

    return 0;
}

