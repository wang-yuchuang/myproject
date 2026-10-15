
#include "iccmsg_common.h"
#include "iccmsg_hal.h"
#include "iccmsg_dpr.h"





typedef enum {
    ICCMSG_DEMO_CMD_RSV = 0,

    IPCC_TX_NACK,
    IPCC_TX_ACK,
    VIRTIO_MULTI_NACK,
    VIRTIO_MULTI_ACK,

    ICCMSG_DEMO_CMD_MAX,
} iccmsg_demo_cmd_t;

typedef struct {
    uint32_t id;
    uint8_t rsv1;
    uint16_t rsv2;
    uint8_t rsv3;
    uint32_t cmd;
}__attribute__ ((packed)) iccmsg_demo_msg_t;




static void usage(void ){
    printf("iccmsg_demo usage:\n");
    printf("-h: print help information.\n");
    printf("-q: selftest iccmsg queue .\n");
    printf("-s <choice>: choose iccmsg send/receive test mode. choice in [0-3].\n");
    printf("\t0: sync send single frame per time.\n");
    printf("\t1: async send single frame per time.\n");
    printf("\t2: sync send multi frame per time.\n");
    printf("\t3: async send multi frame per time.\n");
    printf("-n <ntimes>: iccmsg send/receive test time .\n");

    printf("\n\nFrequently used command\n");
    printf("iccmsg_demo -q ; only test iccmsg_queue.\n");
    printf("iccmsg_demo -s 0 -n 100 ; test sync send single frame 100 times.\n");
}


int main(int argc, char *argv[])
{
    int opt;
    char *rpmsg_dev = NULL;
    int ntimes = 1;
    char ept_dev_path[32];
    int send_test = 0;
    int choice = 0;
    int ret = 0;

    if(argc <= 1)
    {
        usage();
        return -1;
    }

    while ((opt = getopt(argc, argv, "a:d:n:s:bpqxvtr:hi:lm:S:P:")) != -1) {
        switch (opt) {
            case 'q':
                // 消息队列测试
                iccmsg_queue_test();
                break;
            case 's':
                choice = atoi(optarg);
                if(choice > 5){
                    printf("\n unavailable param: -s %d.\n",choice);
                    usage();
                    return -1;
                }
                else{
                    send_test = 1;
                }
                
                break;
            case 'd':
                rpmsg_dev = optarg;
                printf("Using rpmsg device %s\n", rpmsg_dev);
                break;

            case 'n':
                ntimes = atoi(optarg); 
                break;

            case 'h':
                usage();
                break;

            default:
                usage();
                break;
        }
    }
    
    if(send_test){
        iccmsg_hal_test(choice,ntimes);
    }
        
    
    return 0;
}





