/*
 * echo_test.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/*
 * Test application that data integrity of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
 */

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

struct _payload {
    uint32_t magic: 8;
    uint32_t reserved:8;
    uint32_t num : 16;
    uint32_t size;
    char data[];
} __attribute__((__packed__));

#define ECHO_TEST_VERSION "v251213"

#define TEST_MAX_BUF_SIZE   (1024)
#define RPMSG_HEADER_LEN    16
#define TEST_DEFAULT_SIZE   (508 - RPMSG_HEADER_LEN)
#define PAYLOAD_MIN_SIZE    1
#define PAYLOAD_MAX_SIZE    (TEST_DEFAULT_SIZE - sizeof(struct _payload))
#define ECHO_TEST_EPT       (18)
#define RPMSG_BUS_SYS "/sys/bus/rpmsg"

#define IPCC_ACK_LEN        (4)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* parameters */
static int charfd = -1, fd = -1, err_cnt;
static int verbose = 0;
static int benchmark = 0;
static int interval = 0;
static int server_mode;
static int is_socket;
static int rproc_id;
static int sa_addr;
static struct timeval start_test, end_test;
static int setprio;
static int print_flag;

static struct _payload *i_payload;
static struct _payload *r_payload;
/* set default size */
static int rpmsg_mtu_kernel = TEST_DEFAULT_SIZE;
static int rpmsg_payload_test = TEST_DEFAULT_SIZE;
static int payload_max_size = PAYLOAD_MAX_SIZE;
static int num_payloads = (PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE);

#define RPMSG_DEVICE_NAME       "virtio1.rpmsg-echo.-1.30"
static void usage(const char *cmd)
{
    printf("This is s rpmsg echo test %s, MTU=%d\n", ECHO_TEST_VERSION, rpmsg_payload_test);

    printf("command options:\n");
    printf("%s -h\t\t: Show this help info\n", cmd);
    printf("%s -d [devname]\t: Specify device name (default: %s)\n",
            cmd, RPMSG_DEVICE_NAME);
    printf("%s -l\t\t: Show all rpmsg devices in sysfs /sys/bus/rpmsg\n", cmd);
    printf("%s -n [Number]\t: Specify test packet # (default: 1)\n", cmd);
    printf("%s -t [second]\t: Specify test overall duration\n", cmd);
    printf("%s -b\t\t: P2P Lantency test in variable sending bytes\n", cmd);
    printf("%s -p\t\t: Throughput benchmark without acknowledge, like iperf\n", cmd);
    printf("%s -x\t\t: Throughput benchmark with per-send acknowledge (blocking, MTU)\n", cmd);
    printf("%s -i [seconds]\t: test round interval in seconds\n", cmd);
    printf("%s -m [bytes]\t: Specify rpmsg payload size in range of (%ld~%d]\n",
            cmd, sizeof(struct _payload), rpmsg_payload_test);
    printf("%s -S [remote]\t: socket mode to rproc [0:safety 1:secure]\n", cmd);
    printf("%s -a [addr]\t: socket address to rproc app\n", cmd);
    printf("%s -v\t\t: Verbose mode\n", cmd);
    printf("%s -r\t\t: print mode\n", cmd);

    printf("\n\nFrequently used command\n");
    printf("echo_test -l ; to list all devices\n");
    printf("echo_test -d soc:ipcc@0.ipcc-echo.-1.30 ; variable payload test with the device\n");
    printf("echo_test -d soc:ipcc@0.ipcc-echo.-1.30 -b -n 1000 ; Latency test with variable length of payload in 1000 packet average\n");
    printf("echo_test -d soc:ipcc@0.ipcc-echo.-1.30 -p -n 1000 ; Throughput test in average of sending 1000 packets (No Ack)\n");
    printf("echo_test -d soc:ipcc@0.ipcc-echo.-1.30 -x -t 10   ; Throughput test in average (Ack'd) in 10 seconds (print in 1 second)\n");

    exit(0);
}

static void property_usage(const char *cmd)
{
    printf("This is a property test %s\n", ECHO_TEST_VERSION);
    printf("%s set [id] [val]", cmd);
    printf("\t:Set a property [id] with value [val]\n");
    printf("%s get [id]", cmd);
    printf("\t:Get a property value with [id]\n");

    exit(0);
}

long time_elapsed_us(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * (1000000) + end.tv_usec - start.tv_usec;
}

#if 0
static int get_thread_priority(pthread_attr_t *attr)
{
    struct sched_param param;
    int rs = pthread_attr_getschedparam(attr, &param);
    assert(rs == 0);
    printf("priority=%d", param.__sched_priority);
    return param.__sched_priority;
}

static void set_thread_policy(pthread_attr_t *attr, int policy)
{
    int rs = pthread_attr_setschedpolicy(attr, policy);
    assert(rs == 0);
}
#endif

#include <sched.h>
#include <sys/mman.h>

#define MAP_SIZE            4096UL
#define MAP_MASK            (MAP_SIZE -1)
#define SYSCNT_HW_BASE      (0x31400000)
#define SYSCNT_CLK_FREQ     (3000000)
#define SYSCNT_INC_PER_US   (SYSCNT_CLK_FREQ/1000000)

#if 0
static int devmemfd;
static void *devmem_base;
static int syscnt_open_device()
{
    devmemfd = open("/dev/mem", O_RDWR);
    if (devmemfd == -1) {
        fprintf(stderr, "Failed to open /dev/mem errno=%d(%s)\n", -errno, strerror(errno));
        return -1;
    }

    devmem_base = mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, devmemfd, SYSCNT_HW_BASE);
    if (devmem_base == NULL) {
        fprintf(stderr, "Failed to mmap (%d, /dev/mem) errno=%d(%s)\n", devmemfd, -errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int syscnt_close_device()
{
    if (devmem_base)
        munmap(devmem_base, MAP_SIZE);

    if (devmemfd) {
        close(devmemfd);
    }

    return 0;
}

static uint32_t syscnt_get_value(void)
{
    uint64_t *syscnt = devmem_base;

    if (devmem_base)
        return *syscnt & 0xffffffff;

    return 0;
}

static inline uint32_t time_elapse_now(uint32_t start, uint32_t now)
{
    if (now)
        return (now - start) / SYSCNT_INC_PER_US;;

    return 0;
}
#endif

int latency_test(int fd, int round)
{
    int bytes_rcvd, bytes_sent;
    unsigned int rx_count = 0, tx_count = 0;
    long rtt_once, rtt_total, rtt_min, rtt_max;
    struct timeval start_ts, end_ts, txdone_ts;
    char tx_buf[TEST_MAX_BUF_SIZE] = {0,};
    char rx_buf[TEST_MAX_BUF_SIZE] = {0,};
    int len = rpmsg_payload_test;
    struct sched_param sched;
    struct rpmsg_ping_hdr *tx_hdr, *rx_hdr;
    pid_t pid = getpid();
    int flags;

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

    if (setprio) {
        sched.sched_priority = setprio;
        //set_thread_policy(&attr, SCHED_RR);
        sched_setscheduler(pid, SCHED_RR, &sched);
    }

    /* default statistics interval 1 second */
    if (interval == 0)
        interval = 1;

    /* Mark the data buffer. */
    memset(tx_buf, 0xA5, rpmsg_payload_test);

    tx_hdr = (struct rpmsg_ping_hdr *) tx_buf;
    rx_hdr = (struct rpmsg_ping_hdr *) rx_buf;

    tx_hdr->magic = 0xA5;
    tx_hdr->data_len  = len - sizeof(struct rpmsg_ping_hdr);
    tx_hdr->seq = 0;
    //memset(&tx_hdr->time[0], 0, sizeof(tx_hdr->time));

    printf("PING %d(%d) bytes of data\n", len, len + RPMSG_HEADER_LEN);

    rtt_total = 0;
    rtt_min = 100000;
    rtt_max = 0;

    //syscnt_open_device();

    do {
        gettimeofday(&start_ts, NULL);

        /* use the first for the ping sender */
        //tx_hdr->time[0] = syscnt_get_value();
        tx_hdr->seq = tx_count;
        bytes_sent = write(fd, tx_buf, len);
        if (bytes_sent <= 0) {
            perror("\r\n Error sending data");
            break;
        }
        tx_count++;
        gettimeofday(&txdone_ts, NULL);

        bytes_rcvd = read(fd, rx_buf, len);
        if (bytes_rcvd < len) {
            perror("\r\n Error receiving data");
            break;
        }

        gettimeofday(&end_ts, NULL);

        if (tx_hdr->seq != rx_hdr->seq) {
            printf("ping: reply wrong seq # first 16 bytes\n");
            break;
        }

        if (strncmp(rx_hdr->data, tx_hdr->data, tx_hdr->data_len)) {
            printf("ping: result unexpected dump first 16 bytes\n");
            int k;
            /* Validate data buffer integrity. */
            for (k = 0; k < tx_hdr->data_len; k++) {
                if (rx_hdr->data[k] != 0xA5) {

                    if (verbose) {
                        printf("  Data corruption at index %d \r\n", k);
                    }
                    else
                        fprintf(stderr, "X");

                    continue;
                }
            }

            break;
        }

        rx_count++;
        rtt_once = time_elapsed_us(start_ts, end_ts);
        rtt_max  = MAX(rtt_once, rtt_max);
        rtt_min  = MIN(rtt_once, rtt_min);
        rtt_total += rtt_once;


        /* duration test priority to round */
        printf("%d bytes from remote: seq=%d time=%lu/%lu/%lu us\n",
               len, rx_count, time_elapsed_us(start_ts, txdone_ts),
               time_elapsed_us(start_ts, txdone_ts) / 2, rtt_once);
               //time_elapse_now(rx_hdr->time[0], rx_hdr->time[2]), rtt_once);

        memset(rx_buf, 0, len);

        if (tx_count < round)
            sleep(interval);

    } while (tx_count < round);

    printf("%d packets transmitted, %d packets received, %.02f%% loss, time %ld us\n",
           tx_count, rx_count, 100 - 100 * (float)(rx_count/round), rtt_total);
    printf("rtt min/avg/max = %lu/%lu/%lu us\n", rtt_min, rtt_total/rx_count, rtt_max);

    //syscnt_close_device();

    return 0;
}

/*
 * This test the throughput up limit of IPC channel with reliable enhancement,
 * each transmission has ack message confirming the remote does have recieved.
 * it take all hardware capability to transmit data, which likely cause
 * stuck of any function depending on IPC or high latency issue during the test.
 */

int throughput_test(int fd, int round, int seconds)
{
    int bytes_rcvd, bytes_sent;
    unsigned int rx_count = 0, tx_count = 0, rx_count_last = 0;
    long rtt_once, rtt_total, rtt_min, rtt_max;
    long rtt_total_last, rtt_min_last, rtt_max_last;
    struct timeval start_ts, end_ts, last_ts;
    char tx_buf[TEST_MAX_BUF_SIZE] = {0,};
    char rx_buf[IPCC_ACK_LEN] = {0,};
    int len = rpmsg_payload_test;
    struct sched_param sched;
    pid_t pid = getpid();
    int flags;

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

    sched.sched_priority = 20;
//  set_thread_policy(&attr, SCHED_RR);
    sched_setscheduler(pid, SCHED_RR, &sched);

    /* Mark the data buffer. */
    memset(tx_buf, 0xA6, TEST_MAX_BUF_SIZE);

    printf("**************************************\r\n");
    printf("*Throughput Test %d bytes of payload\n", len);
    printf("**************************************\r\n");

    /* default statistics interval 1 second */
    if (interval == 0)
        interval = 1;

    rtt_total = 0;
    rtt_min = 100000;
    rtt_max = 0;

    rtt_total_last = 0;
    rtt_min_last = 100000;
    rtt_max_last = 0;
    rx_count_last = 0;

    gettimeofday(&start_test, NULL);
    last_ts = start_test;

    do {
        gettimeofday(&start_ts, NULL);

        bytes_sent = write(fd, tx_buf, len);
        if (bytes_sent <= 0) {
            perror("\r\n Error sending data");
            break;
        }
        tx_count++;

        bytes_rcvd = read(fd, rx_buf, IPCC_ACK_LEN);
//        while (bytes_rcvd <= 0) {
//            usleep(10);
//            bytes_rcvd = read(fd, rx_buf, IPCC_ACK_LEN);
//        }

        gettimeofday(&end_ts, NULL);

        if (bytes_rcvd != IPCC_ACK_LEN || strncmp(rx_buf, "ACK", bytes_rcvd)) {
            printf("ping: not unexpected ack msg %d\n", bytes_rcvd);
            break;
        }
        rx_count++;
        rtt_once = time_elapsed_us(start_ts, end_ts);
        rtt_max  = MAX(rtt_once, rtt_max);
        rtt_min  = MIN(rtt_once, rtt_min);
        rtt_total += rtt_once;

        rtt_max_last  = MAX(rtt_once, rtt_max_last);
        rtt_min_last  = MIN(rtt_once, rtt_min_last);
        rtt_total_last += rtt_once;
        rx_count_last++;

        /* duration test priority to round */
        if (seconds) {
            long elapsed;

            /* test duration (ms) complete?? */
            gettimeofday(&end_test, NULL);
            elapsed = time_elapsed_us(start_test, end_test)/1000;
            if (elapsed >= 1000 * seconds)
                break;

            elapsed = time_elapsed_us(last_ts, end_test);
            if (elapsed >= interval*1000000) {
                printf("rtt min/avg/max = %lu/%lu/%lu us  %.2f MB/s\n",
                    rtt_min_last, rtt_total_last/rx_count_last, rtt_max_last,
                    (float)(rx_count_last * len)/rtt_total_last);

                last_ts = end_test;
                rtt_total_last = 0;
                rtt_min_last = 100000;
                rtt_max_last = 0;
                rx_count_last = 0;
            }

        } else {
            if (rx_count >= round)
                break;
        }
    } while (1);

    printf("%d packets transmitted, %d acked, %.02f%% loss, time %ld us\n",
                        tx_count, rx_count,
                        100 - 100 * (float)(rx_count/tx_count),
                        rtt_total);
    printf("rtt min/avg/max = %lu/%lu/%lu us\n", rtt_min, rtt_total/rx_count, rtt_max);
    printf("throughput avg. = %.2f MB/s\n", (float)(rx_count * len)/rtt_total);

    return 0;
}

/*
 * This test is like network iperf test for unidirection throughput up limit
 * it take all hardware capability to transmit data, which likely cause
 * stuck of other function depending on IPC or high latency issue during the test.
 */
int iperf_test(int fd, int round)
{
    int j = 0;
    int size, bytes_sent;
    long elapsed;
    struct sched_param sched;
    int count;
    int flags;

    /* Use blocking mode for the test */
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

    err_cnt = 0;
    count = 0;
    size = payload_max_size;
    i_payload = (struct _payload *)malloc(sizeof(struct _payload) + size);

    if (i_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    pid_t pid = getpid();
    sched.sched_priority = 20;
//  set_thread_policy(&attr, SCHED_RR);
    sched_setscheduler(pid, SCHED_RR, &sched);

    /* Mark the data buffer. */
    memset(&(i_payload->data[0]), 0xA5, payload_max_size);
    i_payload->magic = 0xAA;

    printf("**************************************\r\n");
    printf("*iperf Test\n");
    printf("**************************************\r\n");

    i_payload->num = 0;
    i_payload->size = size;
    gettimeofday(&start_test, NULL);

    for (j = 0; j < round; j++) {
        bytes_sent = write(fd, i_payload, sizeof(struct _payload) + size);

        if (bytes_sent <= 0) {
            err_cnt++;
            perror("\r\n Error sending data");
            break;
        }

        count++;

    }

    gettimeofday(&end_test, NULL);

    elapsed = time_elapsed_us(start_test, end_test);

    printf("%d packets transmitted, %d bytes, %d loss\n", count, count * size,
           err_cnt);
    printf("throughput avg. = %.2f MB/s\n", (float)(count * size) / elapsed);
    printf("tx latency avg. = %ld us\n", elapsed / count);

    free(i_payload);

    return 0;
}

int payload_variable_test(int fd, int ntimes, int second)
{
    int i = 0, j = 0;
    int size, bytes_rcvd, bytes_sent;
    char tx_buf[TEST_MAX_BUF_SIZE] = {0,};
    char rx_buf[TEST_MAX_BUF_SIZE] = {0,};
    struct rpmsg_ping_hdr *tx_hdr, *rx_hdr;
    long elapse = 0;

    err_cnt = 0;

    /* Mark the data buffer. */
    memset(tx_buf, 0xA5, rpmsg_payload_test);

    payload_max_size = rpmsg_payload_test - sizeof(struct rpmsg_ping_hdr);

    tx_hdr = (struct rpmsg_ping_hdr *) tx_buf;
    rx_hdr = (struct rpmsg_ping_hdr *) rx_buf;

    tx_hdr->magic = 0xA5;
    tx_hdr->seq = 0;
    memset(&tx_hdr->time[0], 0, sizeof(tx_hdr->time));


    if (second)
        gettimeofday(&start_test, NULL);

    for (j = 0; j < ntimes; j++) {
        printf("\n**************************************\n");
        printf("* Variable Payload Echo Test Round %d\n", j);
        printf("**************************************\n");

        for (i = 0, size = PAYLOAD_MIN_SIZE; size < payload_max_size;
                i++, size++) {
            int k;

            tx_hdr->seq = i;
            tx_hdr->data_len  = size;

            /* Mark the data buffer. */
            memset(&(tx_hdr->data[0]), 0xA5, size);

            if (verbose) printf("\r\n sending payload number");

            if (verbose) printf(" %d of size %lu\r\n", tx_hdr->seq,
                                    (sizeof(struct rpmsg_ping_hdr)) + size);

            bytes_sent = write(fd, tx_hdr, sizeof(struct rpmsg_ping_hdr) + size);

            if (bytes_sent <= 0) {
                if (verbose) {
                    perror("\r\n Error sending data\n");
                    break;
                }
                else
                    fprintf(stderr, "#");

                err_cnt++;
                continue;
            }

            if (verbose) printf("echo test: sent : %d\n", bytes_sent);

            rx_hdr->seq = 0;
            bytes_rcvd = read(fd, rx_buf, rpmsg_payload_test);

            while (bytes_rcvd <= 0) {
                usleep(10000);
                bytes_rcvd = read(fd, rx_buf, rpmsg_payload_test);
            }

            if (verbose) printf(" received payload number ");

            if (verbose) printf("%d of size %d\r\n", rx_hdr->seq, bytes_rcvd);

            /* Validate data buffer integrity. */
            for (k = 0; k < rx_hdr->data_len; k++) {
                if (rx_hdr->data[k] != 0xA5) {
                    err_cnt++;

                    if (verbose) {
                        printf("  Data corruption at index %d \r\n", k);
                        break;
                    }
                    else
                        fprintf(stderr, "X");

                    continue;
                }
            }

            bytes_rcvd = read(fd, rx_buf, rpmsg_payload_test);

            if (!verbose) fprintf(stderr, ".");
        }

        printf("\n**************************************\n");
        printf("* Echo Test Round %d, Results: Error count = %d\n", j, err_cnt);
        printf("**************************************\n");

        if (second) {
            gettimeofday(&end_test, NULL);
            elapse = end_test.tv_sec - start_test.tv_sec;

            if (elapse >= second)
                break;
        }

        if (interval)
            sleep(interval);
    }

    free(i_payload);
    free(r_payload);

    return 0;
}

int server_mode_test(int fd)
{
    int bytes_rcvd, bytes_sent;

    r_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);

    if (r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    while (1) {
        bytes_rcvd = read(fd, r_payload, sizeof(struct _payload) + payload_max_size);

        if (verbose) printf(" received payload number %d of size %d\r\n",
                                r_payload->num, bytes_rcvd);

        bytes_sent = write(fd, r_payload, bytes_rcvd);

        if (verbose) printf(" echo payload %d ?= %d\r\n", bytes_sent, bytes_rcvd);
    }

    free(r_payload);
}

int print_test(int fd)
{
    int bytes_rcvd;
    int i;

    r_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);

    if (r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    while (1) {
        bytes_rcvd = read(fd, r_payload, sizeof(struct _payload) + payload_max_size);
        if(bytes_rcvd > 0 ){
            printf(" received payload number %d of size %d\r\n",
                                r_payload->num, bytes_rcvd);
            for(i = 0; i < bytes_rcvd; i++){
                printf("%c\t",r_payload->data[i]);
            }
        }
        
    }

    free(r_payload);
}

void list_rpmsg_device(void)
{
    int ret = 0;
    ret = system("ls -1 /sys/bus/rpmsg/devices");
    printf("ret = %d\r\n",ret);

    exit(0);
}

void do_property_test(int argc, char **argv)
{
    uint32_t prop_id = 0;
    uint32_t prop_value = 0;
    int ret;

    if (argc == 1) {
        property_usage("property");
    }

    if (strncmp(argv[1], "set", 3) == 0) {
        if (argc != 4) {
            property_usage("property");
        }

        prop_id = strtol(argv[2], NULL, 0);
        prop_value = strtol(argv[3], NULL, 0);
        ret = semidrive_set_domain_property(0, prop_id, prop_value);
        if (ret)
            perror("Failed to create endpoint.\n");
    }

    if (strncmp(argv[1], "get", 3) == 0) {
        if (argc != 3) {
            property_usage("property");
        }

        prop_id = atol(argv[2]);
        ret = semidrive_get_domain_property(0, prop_id, &prop_value);
        if (ret)
            perror("Failed to create endpoint.\n");
    }

    printf("%s property[%d] = 0x%x\n", argv[1], prop_id, prop_value);
}

/*
 * rpmsg socket is experimental feature, not fully tested,
 * not supported by linux upstream, don't use it in the formal module with IPC
 */
int socket_test(int fd, int ntimes, int seconds)
{
    int i = 0, j = 0;
    int size, bytes_rcvd, bytes_sent;
    long elapse = 0;

    err_cnt = 0;
    i_payload = (struct _payload *)malloc(sizeof(struct _payload) + payload_max_size);
    r_payload = (struct _payload *)malloc(sizeof(struct _payload) + payload_max_size);

    if (i_payload == 0 || r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    if (seconds)
        gettimeofday(&start_test, NULL);

    i_payload->magic = 0xA5;

    for (j = 0; j < ntimes; j++) {
        printf("\n**************************************\n");
        printf("* Socket Test Round %d\n", j);
        printf("**************************************\n");
        for (i = 0, size = PAYLOAD_MIN_SIZE; i < num_payloads;
        i++, size++) {
            int k;

            i_payload->num = i;
            i_payload->size = size;

            /* Mark the data buffer. */
            memset(&(i_payload->data[0]), 0xA5, size);

            if (verbose) printf("\r\n sending payload number");
            if (verbose) printf(" %d of size %lu\r\n", i_payload->num,
                            (sizeof(struct _payload)) + size);

            bytes_sent = write(fd, i_payload, sizeof(struct _payload) + size);
            if (bytes_sent <= 0) {
                if (verbose) {
                    perror("\r\n Error sending data\n");
                    break;
                } else
                    fprintf(stderr, "#");

                err_cnt++;
                continue;
            }
            if (verbose) printf("echo test: sent : %d\n", bytes_sent);

            r_payload->num = 0;
            bytes_rcvd = read(fd, r_payload,
                            sizeof(struct _payload) + payload_max_size);
            while (bytes_rcvd <= 0) {
                usleep(10000);
                bytes_rcvd = read(fd, r_payload,
                            sizeof(struct _payload) + payload_max_size);
            }
            if (verbose) printf(" received payload number ");
            if (verbose) printf("%d of size %d\r\n", r_payload->num, bytes_rcvd);

            if (strncmp((const char*)r_payload, "rpcallack", sizeof("rpcallack")) == 0) {
                if (!verbose) fprintf(stderr, ".");
                continue;
            }
            if (strncmp((const char*)r_payload, "rpcallerr", sizeof("rpcallerr")) == 0) {
                err_cnt++;
                fprintf(stderr, "X");
                continue;
            }

            /* Validate data buffer integrity. */
            for (k = 0; k < r_payload->size; k++) {
                if (r_payload->data[k] != 0xA5) {
                    err_cnt++;
                    if (verbose) {
                        printf("  Data corruption at index %d \r\n", k);
                        break;
                    } else
                        fprintf(stderr, "X");
                    continue;
                }
            }
            if (!verbose) fprintf(stderr, ".");
        }
        printf("\n**************************************\n");
        printf("* Echo Test Round %d, Results: Error count = %d\n", j, err_cnt);
        printf("**************************************\n");

        if (seconds) {
            gettimeofday(&end_test, NULL);
            elapse = end_test.tv_sec - start_test.tv_sec;
            if (elapse >= seconds)
                break;
        }

        if (interval)
            sleep(interval);
    }

    free(i_payload);
    free(r_payload);

    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    char *rpmsg_dev = RPMSG_DEVICE_NAME;
    int ntimes = 1;
    struct rpmsg_endpoint_info eptinfo;
    struct rpmsg_endpoint_property ept_prop;
    char ept_dev_path[32];
    int test_second = 0;

    if (strstr(argv[0], "property")) {
        do_property_test(argc, argv);
        return 0;
    }

    if (argc == 1) {
        usage("echo_test");
    }

    while ((opt = getopt(argc, argv, "a:d:n:bpxvstr:hi:lm:S:P:")) != -1) {
        switch (opt) {
            case 'd':
                rpmsg_dev = optarg;
                printf("Using rpmsg device %s\n", rpmsg_dev);
                break;

            case 'n':
                ntimes = atoi(optarg);
                break;

            case 'v':
                verbose = 1;
                break;

            case 'b':
                benchmark = 1;
                break;

            case 'p':
                benchmark = 2;
                break;

            case 'P':
                setprio = atoi(optarg);
                if (setprio < 0 || setprio > 100) {
                    setprio = 0;
                    printf("Set pthread proirity in range [1, 100)\n");
                }
                break;

            case 'x':
                benchmark = 3;
                break;

            case 'S':
                is_socket = 1;
                rproc_id = atoi(optarg);
                break;

            case 'a':
                if (is_socket == 0)
                    usage("echo_test");

                sa_addr = atoi(optarg);
                break;

            case 't':
                test_second = atoi(optarg);
                break;

            case 'i':
                interval = atoi(optarg);
                break;

            case 'm':
                rpmsg_payload_test = atoi(optarg);
                if (rpmsg_payload_test <= sizeof(struct _payload)
                        || rpmsg_payload_test > TEST_MAX_BUF_SIZE) {
                    printf("set max buf size in range (%ld,%d]\n", sizeof(struct _payload),
                           TEST_MAX_BUF_SIZE);
                    exit(-1);
                }

                payload_max_size = (rpmsg_payload_test - sizeof(struct _payload));
                num_payloads = payload_max_size / PAYLOAD_MIN_SIZE;
                break;

            case 'h':
                usage("echo_test");
                break;

            case 'l':
                list_rpmsg_device();
                break;

            case 's':
                server_mode = 1;
                break;

            case 'r':
                print_flag = 1;
                break;

            default:
                printf("getopt return unsupported option: -%c\n", opt);
                usage("echo_test");
                break;
        }
    }

    printf("\r\n Echo test start \r\n");

#if 0
    /* Load rpmsg_char driver */
    printf("\r\nMaster>probe rpmsg_char\r\n");
    ret = system("modprobe rpmsg_char");

    if (ret < 0) {
        perror("Failed to load rpmsg_char driver.\n");
        return -EINVAL;
    }

#endif
    if (is_socket) {
        printf("is_socket \r\n");
        struct sockaddr_rpmsg addr;

        if((fd = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) == -1) {
            perror("Error while opening socket");
            return -1;
        }

        if (sa_addr == 0) {
            printf("please assign sa addr -a [addr]! \r\n");
            return -2;
        }

        addr.family  = AF_RPMSG;
        addr.vproc_id = rproc_id;
        addr.addr = sa_addr;
        if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("Error in socket connect");
            return -2;
        }

        socket_test(fd, ntimes, test_second);

        return 0;
    }

    strcpy(eptinfo.name, "rpmsg-test-channel");
    eptinfo.src = ECHO_TEST_EPT;
    eptinfo.dst = 0xFFFFFFFF;

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        /* managed by rpmsg chrdrv */
        if (rpch_check_path(rpmsg_dev)) {
            return -EINVAL;
        }

        charfd = rpch_create(ept_dev_path, rpmsg_dev, eptinfo);
        if (charfd < 0) {
            return -EINVAL;
        }
    }
    else {
        /* managed by dcf chrdrv */
        strncpy(ept_dev_path, rpmsg_dev, 32);
    }
    
    fd = open(ept_dev_path, O_RDWR | O_NONBLOCK );

    if (fd < 0) {
        perror("Failed to open rpmsg device.");
        close(charfd);
        return -1;
    }
    /* if ioctl not implemented, skip the step */
    if (get_rpmsg_ept_info(fd, &ept_prop) == 0) {
        rpmsg_mtu_kernel = ept_prop.mtu;
        printf("update rpmsg-mtu=%d from kernel\n", rpmsg_mtu_kernel);

        /* test specified payload size should less than kernel mtu */
        if (rpmsg_payload_test > rpmsg_mtu_kernel) {
            rpmsg_payload_test = rpmsg_mtu_kernel;
            payload_max_size = (rpmsg_payload_test - sizeof(struct _payload));
            num_payloads = payload_max_size / PAYLOAD_MIN_SIZE;
        }
    }

    if (server_mode) {
        server_mode_test(fd);
    }
    else if (benchmark == 1) {
        latency_test(fd, ntimes);
    }
    else if (benchmark == 2) {
        iperf_test(fd, ntimes);
    } else if (benchmark == 3) {

        throughput_test(fd, ntimes, test_second);

    } else if(print_flag == 1) {
        printf("print_test start.\n");
        print_test(fd);
    }
    else {
        printf("else.\n");
        if (test_second) {
            ntimes = 0x7fffffff;
        }

        payload_variable_test(fd, ntimes, test_second);
    }

    close(fd);

    if (charfd >= 0)
        close(charfd);

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        unbind_rpmsg_chrdev(rpmsg_dev);
    }

    return 0;
}
