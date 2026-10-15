#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <Can.h>
#include <can_proxy.h>
#include <CLI_type.h>
#include <debug.h>
#include <board.h>

/*
 * Special canid for time test, store the sys count (uint32)from/to can driver
 * into payload of the canframe at offset 4~7 and store the timestamp of
 * linux vircan driver in offset 0~3
 * sys counter is a global clock counter with 3MHz frequency.
 * set CONFIG_VIRCAN_LATENCY_TEST = 1 to enable
 */
#define CONFIG_VIRCAN_LATENCY_TEST  (0)

#if CONFIG_VIRCAN_LATENCY_TEST

#define CAN_ID_TIMESTAMP_TEST   (1U)
#define CAN_ID_MASK             (0x000007FFU) /* standard frame format */

#include "__regs_base.h"
#include "reg.h"
static inline uint32_t get_sys_cnt(void)
{
    return readl(APB_SYS_CNT_RO_BASE);
}
#endif

#define FILTER_RANGE_MODE       (1 << 0)
#define FILTER_BATCH_MASK_MODE      (1 << 1)
#define FILTER_SELECT_ID_MODE       (1 << 2)
#define FILTER_ID_MASK      (0x07FFU)
#define FILTER_RESERVE_MASK         (0x1000)

bool can_filter(struct cp_can_frame *cpframe)
{
    int i;
    int need_reserved = 0;
    u16 can_id = cpframe->can_id;

    if (setfilter.mode & FILTER_RANGE_MODE) {
        if ((can_id > setfilter.id_range_high)
                || (can_id < setfilter.id_range_low)) {
            return false;
        }
    }

    if (setfilter.mode & FILTER_BATCH_MASK_MODE) {
        if (setfilter.batch_id & FILTER_RESERVE_MASK) {
            if ((can_id & setfilter.batch_id) != (setfilter.batch_id &
                                                  FILTER_ID_MASK)) {
                return false;
            }
        }
        else {
            if ((can_id & setfilter.batch_id) == (setfilter.batch_id &
                                                  FILTER_ID_MASK)) {
                return false;
            }
        }
    }

    if (setfilter.mode & FILTER_SELECT_ID_MODE) {
        for (i = 0; i < setfilter.id_count; i++) {
            if (setfilter.can_id[i] & FILTER_RESERVE_MASK) {
                need_reserved = 1;

                if ((can_id & FILTER_ID_MASK) == (setfilter.can_id[i] & FILTER_ID_MASK)) {
                    return true;
                }
            }
            else {
                if ((can_id & FILTER_ID_MASK) == (setfilter.can_id[i] & FILTER_ID_MASK)) {
                    return false;
                }
            }
        }

        if (need_reserved) {
            return false;
        }
    }

    return true;
}

void CanIf_RxIndication(uint16_t ControllerId, Can_IdType CanId,
                        uint8_t CanDlc, const uint8_t *CanSduPtr)
{
    bool ret;
    struct cp_can_frame *cpframe;
    cp_if_t *can_if;
    __CP_CREATE_PFRAME(cp_if_frame_t, CP_CAN_MTU, frame); //64B for CANFD frame
    frame->bus = cpifBusCan;
    frame->if_id = ControllerId;
    frame->len = CanDlc + sizeof(Can_IdType) + 4/*CanDlc + padding*3 */;
    /* pack into cp can frame */
    cpframe = (struct cp_can_frame *) &frame->data[0];
    cpframe->can_id = CanId;
    cpframe->len = CanDlc;
    memcpy(&cpframe->data[0], CanSduPtr, CanDlc);
#if CONFIG_VIRCAN_LATENCY_TEST

    if ((CAN_ID_MASK & CanId) == CAN_ID_TIMESTAMP_TEST) {
        u32 cnt = get_sys_cnt();
        memcpy(&frame->data[sizeof(Can_IdType) + 5], &cnt, sizeof(cnt));
    }

#endif
    CPDBG("Can%d rx <0x%x> [%d] ", ControllerId, cpframe->can_id,
          cpframe->len);

    for (int i = 0; i < cpframe->len; i++) {
        CPDBG("%x ", cpframe->data[i]);
    }

    CPDBG("\n");

    if (setfilter.state == FLT_READY) {
        ret = can_filter(cpframe);

        if (ret == false) {
            CPDBG("fliter out\n");
            goto out;
        }
    }

    can_if = cp_get_interface(cpifBusCan, ControllerId);

    if (can_if->bind_if_id != NO_BIND) {
        CPDBG("can find bind\n");

        if (cp_if_frame_post(frame, DST_AP) == false) {
            CPDBG("incoming queue full, drop!\n");
        }
    }

out:
    return;
}

void CanIf_TxComfirmation(PduIdType CanTxPduId)
{
    cp_if_t *can_if;
    can_if = cp_get_interface(cpifBusCan, CanTxPduId);
    osSemaphoreRelease(can_if->can_state_sem);
}

/* Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo) */
int can_if_tx(struct cp_if_t *cpif, void *from, int len)
{
    uint32_t ret;
    Can_HwHandleType Hth = cpif->if_id * 7;
    /* data 0 ~ 3: id
     * data 4: length
     * data 5 ~ 13: payload
     */
    Can_PduType PduInfo = {0,};
    struct cp_can_frame *cpframe;
    cpframe = from;
    PduInfo.swPduHandle = cpif->if_id;
    PduInfo.length = cpframe->len;
    PduInfo.id = cpframe->can_id;
    PduInfo.sdu = &cpframe->data[0];
#if CONFIG_VIRCAN_LATENCY_TEST

    if ((CAN_ID_MASK & cpframe->can_id) == CAN_ID_TIMESTAMP_TEST) {
        u32 cnt = get_sys_cnt();
        memcpy(&cpframe->data[4], &cnt, sizeof(cnt));
    }

#endif
    CPDBG("Can%d tx <0x%x> [%d] ", cpif->if_id, cpframe->can_id, cpframe->len);

    for (int i = 0; i < cpframe->len; i++) {
        CPDBG("%x ", cpframe->data[i]);
    }

    if (osSemaphoreAcquire(cpif->can_state_sem, 100) != 0) {
        // cpif->if_state = cpifStateStopped;
    }
    ret = Can_Write(Hth, &PduInfo);
    if (ret == E_OK)
        CPDBG("\n");
    else {
        if (ret == E_NOT_OK)
            osSemaphoreRelease(cpif->can_state_sem);
        CPDBG("FAILED!%d\n",ret);
    }

    return 0;
}

int can_if_init(struct cp_if_t *cpif)
{
    extern const Can_ConfigType gCan_Config;
    Can_Init(&gCan_Config);
    Can_SetControllerMode(cpif->if_id, CAN_CS_STARTED);
    cpif->if_state = cpifStateRunning;
    cpif->can_state_sem = osSemaphoreNew(1, 1, NULL);
    return 0;
}

int can_if_rx_cb(struct cp_if_t *cpif, void *data, int len)
{
    cp_bundle_attach(cpif->bind_if_id, data, len);
    return len;
}

#define CAN_PROXY_DEFAULT_ID    (0x500)
#define CAN_PROXY_FRAME_LEN (8)
#define CAN_PROXY_INTERVAL_US   (200)
#define CAN_PROXY_BUS_ID    (1)

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

int do_can_rx_test(int argc, char *argv[])
{
    u32 can_id = 0;
    u32 round = 100;
    u32 i;
    u8 CanSduPtr[CANFD_MAX_DLEN];
    u16 ControllerId = CAN_PROXY_BUS_ID;
    u8 len = CAN_PROXY_FRAME_LEN;
    u32 interval = CAN_PROXY_INTERVAL_US;
    uint64_t start_time, end_time;
    uint64_t rtt_once;
    u64 rx_bytes;
    u64 rx_pkts;

    if (argc == 1) {
        can_id = CAN_PROXY_DEFAULT_ID;
    } else {
        can_id = strtoul(argv[0], NULL, 0);

        if (argc == 3) {
            round = strtoul(argv[1], NULL, 0);
        }

        if (argc == 4) {
            round = strtoul(argv[1], NULL, 0);
            interval = strtoul(argv[2], NULL, 0);
        }

        if (argc == 5) {
            round = strtoul(argv[1], NULL, 0);
            interval = strtoul(argv[2], NULL, 0);
            len = strtoul(argv[3], NULL, 0);
        }
    }

    len = MIN(CANFD_MAX_DLEN, len);
    interval = MIN(interval, 65535);
    printf("canid=0x%x pkts=%d interv=%d len=%d\n", can_id, round, interval,
           len);

    for (i = 0; i < CANFD_MAX_DLEN; i++) {
        CanSduPtr[i] = 0xa0 + i;
    }

    start_time = current_time_hires();
    rx_bytes = 0;
    rx_pkts  = 0;

    for (i = 0; i < round; i++) {
        CanIf_RxIndication(ControllerId, can_id, len, CanSduPtr);
        rx_pkts++;
        rx_bytes += len;
        udelay(interval);
        end_time = current_time_hires();
        rtt_once = end_time - start_time;

        if (rtt_once > 1000000) {
            printf("rx %lld bytes pkt=%lld time=%llu us, %f kbps\n",
                   rx_bytes, rx_pkts, rtt_once, (float)(rx_bytes * 8000) / rtt_once);
            /* reset counters */
            rx_pkts = 0;
            rx_bytes = 0;
            start_time = current_time_hires();
        }
    }

    return 0;
}

static void usage(const char *cmd)
{
    printf("Usage: can proxy unit test\n");
    printf("%s commands:\n\n", cmd);
    printf("%s rx [can_id [pkts [interval [can_dlc]]]]\n", cmd);
    printf("\t\t\t: Simulate receiving several can packets [pkts]\n");
    printf("\t\t\t: Default can_id=0x500, pkts=100, interval=200 us, dlc=8\n");
    printf("Frequently used commands:\n");
    printf("%s rx\n", cmd);
    printf("%s rx 0x304 100000 125 8\n", cmd);
    printf("%s rx 0x513 100000 100 64\n", cmd);
}

static int do_can_proxy_test(int argc, char *argv[])
{
    if (argc < 1) {
        usage("do_can_proxy_test");
        return 0;
    }

    if (!strcmp(argv[0], "rx")) {
        do_can_rx_test(argc, &argv[1]);
    }

    return 0;
}

CLI_CMD("do_can_proxy_test", "can proxy unit test", do_can_proxy_test);