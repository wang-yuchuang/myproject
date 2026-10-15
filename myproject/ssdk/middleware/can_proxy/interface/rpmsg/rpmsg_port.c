#include <malloc.h>
#include <param.h>
#include <rpmsg_channel.h>
#include <rpmsg_echo.h>
#include <rpmsg_port.h>
#include <can_proxy.h>
#include <spinlock.h>
#include <rpc/dcf_rpc.h>
#include <debug.h>
#include <string.h>
#include <rpmsg_service.h>
#include <sd_errcode.h>

#define RPMSG_PORT_VERSION (0x1001)
#define NO_IF (MAX_INTERFACE_PER_BUS + 1)

#define CP_BAUDRATE_250K_500K (1)
#define CP_BAUDRATE_500K_1M (2)
#define CP_BAUDRATE_500K_2M (3)
#define CP_BAUDRATE_1M_2M (4)
#define CP_BAUDRATE_1M_5M (5)
#define DP_CR5_MPC 2

typedef struct cp_if_rpmsg_handle_t {
    rpmsg_channel_t *chan;
    const char *name;
    uint16_t srv_id;
    uint16_t ept_id;
    int rproc;
} cp_if_rpmsg_handle_t;


enum sts_op_type {
    CREATE_CHANNEL = 0,
    CP_OP_GET_VERSION,
    CP_OP_GET_CONFIG,
    CP_OP_START,
    CP_OP_STOP,
    SET_FILTER,
};

/* Do not exceed 32 bytes so far */
struct canctl_cmd {
    u16 op;
    u16 tag;
    union {
        struct {
            u16 endpoint;
            u16 canif_bind;
        } s;
        struct {
            u16 mode;
            u16 id_range_high;
            u16 id_range_low;
            u16 batch_id;
            u16 id_count;
            u16 can_id[4];
        } f;
    } msg;
};

/* Do not exceed 16 bytes so far */
struct canctl_result {
    u16 op;
    u16 tag;
    union {
        /** used for get_version */
        struct {
            u16 version;
            u16 id;
            u16 vendor;
        } v;
        /** used for get_config */
        struct {
            u8 baudrate;
            u8 canfd_support;
            u8 instances;
            u8 padding;
        } cfg;
        u8 data[12];
    } msg;
};

static spin_lock_t table_lock;
static rpmsg_if_map_t rpmsg_if_tlb[MAX_INTERFACE_PER_BUS] = {0,};
struct vircan_setfilter setfilter;

static uint32_t rpmsg_chan2if_id(void *data, unsigned long ept)
{
    uint32_t rproc;

    rpmsg_channel_get_type_rproc(data, NULL, &rproc);

    for (uint32_t i = 0;
            i < sizeof(rpmsg_if_tlb) / sizeof(rpmsg_if_tlb[0]); i++) {
        if (rpmsg_if_tlb[i].port.ept_id == ept &&
                rpmsg_if_tlb[i].port.rproc == rproc) {
            return rpmsg_if_tlb[i].if_id;
        }
    }

    return NO_IF;
}

static uint32_t rpmsg_port2if_id(int rproc, int endpoint)
{
    for (uint32_t i = 0;
            i < sizeof(rpmsg_if_tlb) / sizeof(rpmsg_if_tlb[0]); i++) {
        if (rpmsg_if_tlb[i].port.ept_id == endpoint &&
                rpmsg_if_tlb[i].port.rproc == rproc) {
            return rpmsg_if_tlb[i].if_id;
        }
    }

    return NO_IF;
}

static int rpmsg_common_cb(void *data, int len, unsigned long src, void *arg)
{
    cp_if_frame_t *frame = malloc(sizeof(cp_if_frame_t) + len);
    ASSERT(frame);
    frame->bus = cpifBusRPMSG;
    frame->if_id = rpmsg_chan2if_id(arg, src);
    frame->len = len;
    memcpy(frame->data, data, len);
    CPDBG("==== RPMSG recv data %d from ept %lu==== ", len, src);

    for (int i = 0; i < len; i++) {
        CPDBG("0x%x ", frame->data[i]);
    }

    CPDBG("\n");
    cp_if_frame_post(frame, DST_SAF);
    free(frame);
    return 0;
}

int rpmsg_if_init(struct cp_if_t *cpif)
{
    cp_if_rpmsg_handle_t *handle = malloc(sizeof(cp_if_rpmsg_handle_t));
    struct rpmsg_instance *dev;
    int ret = 0;

    if (!handle) {
        CPDBG("rpmsg interface malloc fail\n");
        free(handle);
        return ERR_NO_MEMORY;
    }

    rpmsg_cfg_container_t *cc = cpif->container;
    handle->ept_id = cc->ept_id;
    handle->name = cc->name;
    handle->rproc = cc->rproc;

    ssdk_printf(SSDK_CRIT, "%s: %p rproc:%d ept:%d\n", __func__, handle->name, handle->rproc, handle->ept_id);
    /* get a rpmsg instance */
    if (handle->rproc == DP_CR5_MPC) {
        dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, handle->rproc);
    } else {
        dev = rpmsg_get_instance(RPMSG_TYPE_VIRTIO, handle->rproc);
    }
    if (!dev) {
        ERRCODE_PRINT(MOD_ID_CANFD, cc->ept_id, CAN_ERR_NO_RPDEV, 0U);
        ssdk_printf(SSDK_CRIT,
                    "rpmsg_if_init: failed to create an rpmsg device\n");
        return ERR_NOT_READY;
    }

    /* create a rpmsg channel */
    handle->chan = rpmsg_channel_create(dev, handle->name,
                           handle->ept_id);
    if (handle->chan == NULL) {
        ERRCODE_PRINT(MOD_ID_CANFD, cc->ept_id, CAN_ERR_NO_RPCHAN, 0U);
        ssdk_printf(SSDK_CRIT, "rpmsg_if_init: failed to create rpmsg channel\n");
        return ERR_NOT_VALID;
    }

    /* start the rpmsg channel */
    ret = rpmsg_channel_start(handle->chan, rpmsg_common_cb,
                              handle->chan);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "rpmsg_if_init: start rpmsg channel failed.\n");
        rpmsg_channel_destroy(handle->chan);
        return ERR_FAULT;
    }

    cpif->container = handle;
    cpif->ctn_size = sizeof(cp_if_rpmsg_handle_t);
    cpif->if_state = cpifStateInited;
    return ret;
}

static int generate_bundle_rules(int bind_can_if_id, int channel_id)
{
    struct cp_if_t *can_if = NULL;
    can_if = cp_get_interface(cpifBusCan, bind_can_if_id);

    if (can_if->bind_if_id == NO_BIND) {
        can_if->bind_if_id = TARGET_VIRCANx_BUNDLE(channel_id);
    }
    else {
        can_if->bind_if_id |= TARGET_VIRCANx_BUNDLE(channel_id);
    }

    CPDBG("generate bundle rules %x", can_if->bind_if_id);
    return 0;
}

static uint32_t allocate_channel(rpc_server_handle_t hserver,
                                 struct canctl_cmd *ctl)
{
    uint32_t ret = NO_IF;
    uint32_t rproc;

    rpmsg_channel_get_type_rproc(hserver->rpchan, NULL, &rproc);

    for (uint32_t i = 0; i < MAX_INTERFACE_PER_BUS; i++) {
        if (rpmsg_if_tlb[i].chan_inited != true) {
            rpmsg_if_tlb[i].chan_inited = true;
            rpmsg_if_tlb[i].if_id = i;
            rpmsg_if_tlb[i].port.ept_id = ctl->msg.s.endpoint;
            rpmsg_if_tlb[i].port.name = "rpmsg-vircan";
            rpmsg_if_tlb[i].port.rproc = rproc;
            printf("rpmsg_if_tlb if_id is %d rproc is %d\n", rpmsg_if_tlb[i].if_id,
                   rpmsg_if_tlb[i].port.rproc);
            ret = rpmsg_if_tlb[i].if_id;
            break;
        }
    }

    return ret;
}

static rpc_call_result_t rpmsg_ctl_cb(rpc_server_handle_t hserver,
                                      rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct canctl_cmd *ctl = NULL;
    struct canctl_result *r = (struct canctl_result *)&result.result[0];
    result.ack = request->cmd;
    struct cp_if_t *rpmsg_if = NULL;
    int bind_can_if_id = 0;
    uint32_t rpmsg_if_id = 0;
    uint32_t rproc;

    rpmsg_channel_get_type_rproc(hserver->rpchan, NULL, &rproc);

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        ssdk_printf(SSDK_CRIT, "%s: invalid request from %x\n", __func__,
                MOD_RPC_REQ_CP_IOCTL);
        return result;
    }

    ctl = (struct canctl_cmd *)&request->param[0];

    switch (ctl->op) {
        case CREATE_CHANNEL:
            spin_lock(&table_lock);
            rpmsg_if_id = rpmsg_port2if_id(rproc, ctl->tag);

            if (rpmsg_if_id != NO_IF) {
                CPDBG("already create the channel\n");
                result.retcode = 0;
                spin_unlock(&table_lock);
                break;
            }

            rpmsg_if_id = allocate_channel(hserver, ctl);

            if (rpmsg_if_id == NO_IF) {
                printf("interface out of range\n");
                result.retcode = -1;
                spin_unlock(&table_lock);
                break;
            }

            cp_interface_setting(cpifBusRPMSG,
                                 rpmsg_if_id,
                                 rpmsg_if_init,
                                 rpmsg_if_tx,
                                 rpmsg_if_rxcb,
                                 (void *)&rpmsg_if_tlb[rpmsg_if_id].port, sizeof(rpmsg_cfg_container_t));

            spin_unlock(&table_lock);
            rpmsg_if = cp_get_interface(cpifBusRPMSG, rpmsg_if_id);

            if (rpmsg_if_init(rpmsg_if)) {
                printf("init %d:%d cp interface fail.\n", cpifBusRPMSG,
                       rpmsg_if_id);
                // xLocalCPIFS.if_id_bitmap[cpifBusRPMSG] &= ~(INTERFACE_ID_BITMAP(rpmsg_if_id));
                result.retcode = ERR_CHANNEL_CLOSED;
                break;
            }

            bind_can_if_id = ctl->msg.s.canif_bind;
            rpmsg_if->bind_if_id = TARGET_CANx_BUNDLE(bind_can_if_id);
            generate_bundle_rules(bind_can_if_id, rpmsg_if->if_id);
            result.retcode = 0;
            break;

        case SET_FILTER:
            if (ctl->msg.f.mode == 0) {
                memset(&setfilter, 0, sizeof(struct vircan_setfilter));
                setfilter.state = FLT_NOTSUP;
            }
            else {
                setfilter.mode = ctl->msg.f.mode;
                setfilter.id_range_high = ctl->msg.f.id_range_high;
                setfilter.id_range_low = ctl->msg.f.id_range_low;
                setfilter.batch_id = ctl->msg.f.batch_id;
                setfilter.id_count = ctl->msg.f.id_count;

                for (int i = 0; i < setfilter.id_count; i++) {
                    setfilter.can_id[i] = ctl->msg.f.can_id[i];
                }

                setfilter.state = FLT_READY;
            }

            break;

        case CP_OP_GET_VERSION:
            // TODO: read from can driver
            r->msg.v.version = RPMSG_PORT_VERSION;
            r->msg.v.id = 0;
            r->msg.v.vendor = 0x0;
            result.retcode = 0;
            break;

        case CP_OP_GET_CONFIG:
            // TODO: read from can driver
            r->msg.cfg.baudrate = CP_BAUDRATE_1M_5M;
            r->msg.cfg.canfd_support = 1;
            r->msg.cfg.instances = MAX_INTERFACE_PER_BUS;
            result.retcode = 0;
            break;

        case CP_OP_START:
        case CP_OP_STOP:
            rpmsg_if_id = rpmsg_port2if_id(rproc, ctl->tag);

            if (rpmsg_if_id == NO_IF) {
                result.retcode = ERR_NOT_VALID;
                ssdk_printf(SSDK_CRIT, "%s:Bad instance=%d\n", __func__, ctl->tag);
                return result;
            }

            rpmsg_if = cp_get_interface(cpifBusRPMSG, rpmsg_if_id);

            if (rpmsg_if->if_state == cpifStateUnkown) {
                result.retcode = ERR_NOT_READY;
                ssdk_printf(SSDK_CRIT, "%s:instance=%d not ready\n", __func__, ctl->tag);
                return result;
            }

            if (rpmsg_if->if_state == cpifStateStopped
                    || (rpmsg_if->if_state == cpifStateInited)) {
                rpmsg_if->if_state = cpifStateRunning;
                ssdk_printf(SSDK_CRIT, "%s:rpmsg_if=%d, started\n", __func__, rpmsg_if_id);
            }
            else {
                rpmsg_if->if_state = cpifStateStopped;
                ssdk_printf(SSDK_CRIT, "%s:rpmsg_if=%d, stopped\n", __func__, rpmsg_if_id);
            }

            result.retcode = 0;
            break;

        default:
            result.retcode = ERR_INVALID_ARGS;
            ssdk_printf(SSDK_CRIT, "%s:rpmsg_if=%d, op err\n", __func__, ctl->tag);
            break;
    }

    ssdk_printf(SSDK_INFO, "%s:rpmsg_if=%d, cmd=%d done\n", __func__, rpmsg_if_id,
            ctl->op);
    return result;
}

int rpmsg_if_start_manager(void)
{
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_CP_IOCTL, rpmsg_ctl_cb, IPCC_RPC_NO_FLAGS},
    };
    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));
    spin_lock_init(&table_lock);
    return 0;
}

int rpmsg_if_rxcb(struct cp_if_t *cpif, void *data, int len)
{
    cp_bundle_attach(cpif->bind_if_id, data, len);
    return len;
}

int rpmsg_if_tx(struct cp_if_t *cpif, void *from, int len)
{
    cp_if_rpmsg_handle_t *handle = cpif->container;
    struct cp_can_frame *cpframe = (struct cp_can_frame *)from;

    if (cpif->if_state != cpifStateRunning)
        return -1;

    CPDBG("==== RPMSG send data %d ====\n", len);

    for (int i = 0; i < cpframe->len; i++) {
        CPDBG("0x%x ", cpframe->data[i]);
    }

    CPDBG("\n");
    /* Send the entire cp can frame */
    return rpmsg_channel_send(handle->chan, handle->ept_id, (char *)from, CP_CAN_MTU, 1000);
}