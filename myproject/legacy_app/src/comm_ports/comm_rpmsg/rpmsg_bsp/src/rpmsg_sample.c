#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/rpmsg.h>
#include <pthread.h>
#include <rpch_common.h>

#define RPMSG_SAMPLE_VERSION           "v211215"

#define RPMSG_DEVICE_NAME           "soc:ipcc@0.myendpoint.-1.500"
#define RPMSG_SAMPLE_EPT            (501)

#define SAMPLE_SEND_TIMES   (10)

// different test case command
enum saf_cmd_type {
    CMD_REPLY_DATA = 0,
    CMD_SEND_DATA,
    CMD_CLOSE_CHANNEL,
    CMD_SEND_DATA_NOB,
    CMD_RECV_DATA_NOB,
};

#define TEST_OVER (-1)

static void usage(const char *cmd)
{
    printf("This is a rpmsg sample %s, MTU=%d\n", RPMSG_SAMPLE_VERSION, MAX_RPMSG_BUFF_SIZE);
    printf("command options:\n");

    printf("%s -h\t\t: Show this help info\n", cmd);
    printf("%s -l\t\t: Show all rpmsg devices in sysfs /sys/bus/rpmsg\n", cmd);
    printf("%s -d [devname]\t: Specify device name (default: %s)\n",
            cmd, RPMSG_DEVICE_NAME); // no need to change device name

    // different test case
    printf("%s -s\t\t: Send data and wait for a response\n", cmd);
    printf("%s -r\t\t: Send request and wait for recieving data\n", cmd);
    printf("%s -c\t\t: Send request for closing channel\n", cmd);
    printf("%s -b\t\t: Send request and wait for recieving data with nonblock mode\n", cmd);
    printf("%s -i\t\t: Send request to make remote recieving data with nonblock mode\n", cmd);
    exit(0);
}

void list_rpmsg_device(void)
{
    system("ls -1 /sys/bus/rpmsg/devices");
    exit(0);
}

static int charfd = -1;
static int fd = -1;

// test case 0
int send_test(int fd)
{
    int bytes_sent;
    int bytes_rcvd;
    char tx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    char rx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    int len = MAX_RPMSG_BUFF_SIZE;
    struct _sample_payload *tx_hdr = (struct _sample_payload *) tx_buf;
    tx_hdr->cmd = CMD_REPLY_DATA;
    tx_hdr->size  = PAYLOAD_SIZE;

    bytes_sent = write(fd, tx_buf, len);
    if (bytes_sent <= 0) {
        perror("\r\n Error sending data");
    }

    bytes_rcvd = read(fd, rx_buf, len);
    if (bytes_rcvd < len) {
        perror("\r\n Error receiving data");
    }
    else {
        printf("recieve the data %d bytes\n",bytes_rcvd);
    }

    return 0;
}

// test case 2
int recv_test(int fd)
{
    int bytes_sent;
    int bytes_rcvd;
    char tx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    char rx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    int len = MAX_RPMSG_BUFF_SIZE;
    struct _sample_payload *tx_hdr = (struct _sample_payload *) tx_buf;
    struct _sample_payload *rx_hdr = (struct _sample_payload *) rx_buf;
    tx_hdr->cmd = CMD_SEND_DATA;
    tx_hdr->size  = PAYLOAD_SIZE;
    char message[] = "hello safety";
    memcpy(tx_hdr->data, message, sizeof(message));

    bytes_sent = write(fd, tx_buf, len);
    if (bytes_sent <= 0) {
        perror("\r\n Error sending data");
    }

    bytes_rcvd = read(fd, rx_buf, len);
    if (bytes_rcvd < len) {
        perror("\r\n Error receiving data");
    }
    else {
        printf("the message is: %s\n", rx_hdr->data);
    }

    return 0;
}

// test case 3
int close_channel_test(int fd)
{
    int bytes_sent;
    char tx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    int len = MAX_RPMSG_BUFF_SIZE;
    struct _sample_payload *tx_hdr = (struct _sample_payload *) tx_buf;
    tx_hdr->cmd = CMD_CLOSE_CHANNEL;
    tx_hdr->size  = PAYLOAD_SIZE;

    bytes_sent = write(fd, tx_buf, len);
    if (bytes_sent <= 0) {
        perror("\r\n Error sending data");
    }

    return 0;
}

// test case 4
int nonblock_test(int fd)
{
    int bytes_sent;
    int bytes_rcvd;
    int recv_times;
    int rcvd_times = 0;
    int flags;
    char tx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    char rx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    int len = MAX_RPMSG_BUFF_SIZE;
    struct _sample_payload *tx_hdr = (struct _sample_payload *) tx_buf;
    struct _sample_payload *rx_hdr = (struct _sample_payload *) rx_buf;
    tx_hdr->cmd = CMD_SEND_DATA_NOB;
    tx_hdr->size  = PAYLOAD_SIZE;
    tx_hdr->arg2 = SAMPLE_SEND_TIMES;
    char message[] = "hello safety, send 10 messages";
    memcpy(tx_hdr->data, message, sizeof(message));

    bytes_sent = write(fd, tx_buf, len);
    if (bytes_sent <= 0) {
        perror("\r\n Error sending data");
    }

    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
        printf("fcntl(%d, F_GETFL)", fd);
        return -1;
    }
    if (!(flags & O_NONBLOCK)) {
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            printf("fcntl(%d, F_SETFL)", fd);
            return -1;
        }
    }

    while (1) {
        bytes_rcvd = read(fd, rx_buf, len);
        if (bytes_rcvd < len) {
            continue;
        }
        else {
            recv_times = rx_hdr->arg2;
            if (rx_hdr->arg1 == 0) {
                printf("the message is: %s\n", rx_hdr->data);
            }
            printf("the data is: %d\n", rx_hdr->arg1);
            rcvd_times ++;
        }
        if (rcvd_times == recv_times) {
            break;
        }
    }

    return 0;
}

// test case 5
int set_nonblock_test(int fd)
{
    int bytes_sent;
    int bytes_rcvd;
    int send_times;
    char tx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    char rx_buf[MAX_RPMSG_BUFF_SIZE] = {0,};
    int len = MAX_RPMSG_BUFF_SIZE;
    struct _sample_payload *tx_hdr = (struct _sample_payload *) tx_buf;
    struct _sample_payload *rx_hdr = (struct _sample_payload *) rx_buf;
    tx_hdr->cmd = CMD_RECV_DATA_NOB;
    tx_hdr->size  = PAYLOAD_SIZE;
    tx_hdr->arg2 = 0; // set timeout
    char message[] = "hello safety, recv data with nonblocking mode";
    memcpy(tx_hdr->data, message, sizeof(message));

    bytes_sent = write(fd, tx_buf, len);
    if (bytes_sent <= 0) {
        perror("\r\n Error sending data");
    }

    bytes_rcvd = read(fd, rx_buf, len);
    if (bytes_rcvd < len) {
        perror("\r\n Error receiving data");
    }
    else {
        printf("the message is: %s\n", rx_hdr->data);
        send_times = rx_hdr->arg2;
        for (int a = 0;a <= send_times;a++)
        {
            if (a == send_times) {
                tx_hdr->cmd = TEST_OVER;
            }
            tx_hdr->size  = PAYLOAD_SIZE;
            tx_hdr->arg1 = a;
            bytes_sent = write(fd, tx_buf, len);
            if (bytes_sent <= 0) {
                perror("\r\n Error sending data");
            }
            sleep(1);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    char *rpmsg_dev = RPMSG_DEVICE_NAME;
    struct rpmsg_endpoint_info eptinfo;
    char *ept_dev_path = (char *)malloc(32);
    int benchmark = 0;

    while ((opt = getopt(argc, argv, "hld:srcbi")) != -1) {
        switch (opt) {
            case 'h':
                usage("rpmsg_sample");
                break;

            case 'l':
                list_rpmsg_device();
                break;

            case 'd':
                rpmsg_dev = optarg;
                break;

            case 's':
                benchmark = 1;
                break;

            case 'r':
                benchmark = 2;
                break;

            case 'c':
                benchmark = 3;
                break;

            case 'b':
                benchmark = 4;
                break;

            case 'i':
                benchmark = 5;
                break;

            default:
                printf("getopt return unsupported option: -%c\n", opt);
                usage("echo_test");
                break;
        }
    }

    strcpy(eptinfo.name, "rpmsg-sample-channel");
    eptinfo.src = RPMSG_SAMPLE_EPT;
    eptinfo.dst = 0xFFFFFFFF;

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        /* managed by rpmsg chrdrv */
        if (rpch_check_path(rpmsg_dev))
            return -EINVAL;

        charfd = rpch_create(ept_dev_path, rpmsg_dev, eptinfo);
        if (charfd < 0) {
            return -EINVAL;
        }
    }
    else {
        /* managed by dcf chrdrv */
        strncpy(ept_dev_path, rpmsg_dev, 32);
    }

    fd = open(ept_dev_path, O_RDWR);
    if (fd < 0) {
        perror("Failed to open rpmsg device.");
        close(charfd);
        return -1;
    }

    if (benchmark == 1) {
        send_test(fd);
    }

    if(benchmark == 2) {
        recv_test(fd);
    }

    if(benchmark == 3) {
        close_channel_test(fd);
    }

    if(benchmark == 4) {
        nonblock_test(fd);
    }

    if(benchmark == 5) {
        set_nonblock_test(fd);
    }

    close(fd);
    if (charfd >= 0)
        close(charfd);
    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        unbind_rpmsg_chrdev(rpmsg_dev);
    }

    return 0;
}
