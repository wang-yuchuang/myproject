#ifndef __CAN_PROXY_H__
#define __CAN_PROXY_H__

#include <stdio.h>
#include <stdint.h>
#include <cmsis_os2.h>

#include "can_proxy_buffer.h"
#include "can_proxy_bundle.h"

#define CPDBG_EN    0

#if CPDBG_EN
#define CPDBG(...) printf(__VA_ARGS__)
#else
#define CPDBG(...)
#endif

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

/* port declaration */

/* can */

/* CAN Frame Max Data Length */
#define CANFD_MAX_DLEN          (64U)
/* CAN Frame Type(0:DATA or 1:REMOTE). */
#define CP_CAN_FLAG_TYPE        (1 << 29)
/* CAN FD or classic frame? 0: classic or 1: canfd */
#define CP_CAN_FLAG_CANFD       (1 << 30)
/* CAN Frame Identifier(0: STD or 1: EXT format). */
#define CP_CAN_FLAG_FMT         (1 << 31)
struct cp_can_frame {
    u32 can_id;  /* 32 bit CAN_ID + FORMAT/CANFD/TYPE flags */
    u8    len;     /* frame payload length in byte */
    u8    flags;
    u8    res0;
    u8    res1;
    u8    data[CANFD_MAX_DLEN];
} __attribute__((aligned(8)));

#define CP_CAN_MTU  (sizeof(struct cp_can_frame))

struct vircan_setfilter {
    u16 state;
    u16 mode;
    u16 id_range_high;
    u16 id_range_low;
    u16 batch_id;
    u16 id_count;
    u16 can_id[4];
};

enum filter_state {
    FLT_NOTSUP,
    FLT_READY,
};

enum can_state {
    IDLE,
    BUSY,
};

int can_if_tx(struct cp_if_t *cpif, void *from, int len);
int can_if_init(struct cp_if_t *cpif);
int can_if_rx_cb(struct cp_if_t *cpif, void *data, int len);
void start_can_proxy(void);

/* rpmsg */

int rpmsg_if_init(struct cp_if_t *cpif);
int rpmsg_if_rxcb(struct cp_if_t *cpif, void *data, int len);
int rpmsg_if_tx(struct cp_if_t *cpif, void *from, int len);

/* Bundle Rule generation */

//bundle_id = bus_id.if_id
// | bus 3 | bus 2 | ... |FIXED_POINT|if 3 | if 2 | ... | if 0|
// | bit 31| bit 30|... |FIXED_POINT |bit 3| bit 2| ... | bit 0|
#define GEN_BUNDLE_ID(bus_bitmap,if_bitmap) (bus_bitmap | if_bitmap)
#define REV_BUNDLE_ID(bundle_id,bus_bitmap,if_bitmap) do {\
    bus_bitmap = bundle_id & (0xffffffff << BITMAP_FIXED_POINT); \
    if_bitmap = bundle_id & (0xffffffff >> (31 - BITMAP_FIXED_POINT) ); \
}while(0)

/* bundles definition */
/* A bundle that send to can x interface. Usually comes from rpmsg. */
#define TARGET_CANx_BUNDLE(if_id)   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusCan),INTERFACE_ID_BITMAP(if_id))

/* A bundle that send to vircanx. Usually comes from can/spi. */
#define TARGET_VIRCANx_BUNDLE(if_id)   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusRPMSG),INTERFACE_ID_BITMAP(if_id))

extern struct vircan_setfilter setfilter;
#endif