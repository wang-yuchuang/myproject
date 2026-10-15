/*
 * str_req.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

//#include <assert.h>
//#include <dirent.h>
//#include <errno.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include <string.h>
//#include <unistd.h>
//#include <time.h>
//#include <fcntl.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/ioctl.h>
//#include <sys/time.h>
//#include <linux/rpmsg.h>
//#include <pthread.h>
#include <rpch_common.h>

static int charfd = -1, fd = -1;

#define STR_TEST_EPT         (241)
#define RPMSG_DEVICE_NAME    "soc:ipcc@0.str-manage.-1.241"
#define MSG                  "request_str"

int send_msg(int fd, char * msg)
{
    //char * buf = NULL;
    //int bytes_rcvd = 0;
    int flags;
    //char rx_buf[32] = {0,};
    //struct sched_param sched;
    //pid_t pid = getpid();

    if (NULL == msg) {
        printf("ERROR: mesg NULL\n");
        return -1;
    }

    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
        printf("fcntl(%d, F_GETFL)", fd);
        return -1;
    }

    if (flags & O_NONBLOCK) {
        if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
            printf("fcntl(%d, F_SETFL)", fd);
            return -1;
         }
    }
    //memset(rx_buf, 0x00, 32);

    //printf("send(%s), len(%d)\n", msg, strlen(msg));

    write(fd, msg, strlen(msg));

    //sched.sched_priority = 20;
    //sched_setscheduler(pid, SCHED_RR, &sched);
#if 0
    bytes_rcvd = 0;
    while (bytes_rcvd <= 0) {
        usleep(10000);
        bytes_rcvd = read(fd, rx_buf, 16);
    }
    //bytes_rcvd = read(fd, buf, 14);
    printf("receive msg(%s),len(%d)\n",rx_buf,bytes_rcvd);

    if (0 == strncmp(rx_buf,"suspend_to_ram",14)) {
        printf("will power off\n");
        if(-1 == execl("/system/bin/input","input","keyevent","26",NULL)) {
            printf("input keyevent 26 error\n");
        }
    }

    //free(buf);
#endif
    return 0;
}

int main(int argc, char *argv[])
{
    char *rpmsg_dev = RPMSG_DEVICE_NAME;
    char *msg = MSG;
    struct rpmsg_endpoint_info eptinfo;
    char ept_dev_path[32];
    printf("\r\n request str start \r\n");

    strcpy(eptinfo.name, "str-req-channel");
    eptinfo.src = STR_TEST_EPT;
    eptinfo.dst = 0xFFFFFFFF;

    if (rpch_check_path(rpmsg_dev)) {
        return -EINVAL;
    }

    charfd = rpch_create(ept_dev_path, rpmsg_dev, eptinfo);
    if (charfd < 0) {
        return -EINVAL;
    }

    fd = open(ept_dev_path, O_RDWR | O_NONBLOCK );

    if (fd < 0) {
        perror("Failed to open rpmsg device.");
        close(charfd);
        return -1;
    }

    send_msg(fd, msg);

    close(fd);

    if (charfd >= 0)
        close(charfd);

    unbind_rpmsg_chrdev(rpmsg_dev);
    return 0;
}

