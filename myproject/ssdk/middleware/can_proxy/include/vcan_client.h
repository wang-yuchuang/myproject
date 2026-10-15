#ifndef __VCAN_CLIENT_H__
#define __VCAN_CLIENT_H__

#include <string.h>
#include <driver/can.h>
#include <rpmsg_channel.h>

#define AF_RPMSG 44

#ifndef _def_domain_cpu_id_t
#define _def_domain_cpu_id_t
typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
#if (defined ULINK_SIDE_A) || (defined ULINK_SIDE_B)
    R_DP_CR5_SAF = DP_CPU_MAX,
    R_DP_CR5_SEC,
    R_DP_CR5_MPC,
    R_DP_CA_AP1,
    R_DP_CA_AP2,
    R_DP_DSP_V,
    ALL_DP_CPU_MAX,
#endif
} domain_cpu_id_t;
#endif

struct dcf_front_device {
	const char *name;
	int can_bind;
	osMutexId_t ept_lock;
	int sock;
	rpmsg_channel_t *chan;
	uint16_t srv_id;
	uint16_t ept_id;
	int rproc;
};

enum sts_op_type {
	CREATE_CHANNEL,
	CP_OP_GET_VERSION,
	CP_OP_GET_CONFIG,
	CP_OP_START,
	CP_OP_STOP,
	CP_OP_SET_FILTER,
};

struct canctl_cmd {
	uint16_t op;
	uint16_t tag;
	union {
		struct {
			uint16_t endpoint;
			uint16_t canif_bind;
		} s;
		struct {
			uint16_t mode;
			uint16_t id_range_high;
			uint16_t id_range_low;
			uint16_t batch_id;
			uint16_t id_count;
			uint16_t can_id[];
		} f;
	} msg;
};

struct canctl_result {
	uint16_t op;
	uint16_t tag;
	union {
		struct {
			uint16_t version;
			uint16_t id;
			uint16_t vendor;
		} v;
		struct {
			uint8_t baudrate;
			uint8_t canfd_support;
			uint8_t instances;
			uint8_t padding;
		} cfg;
		uint8_t data[12];
	} msg;
};

int vcan_init(struct dcf_front_device *fdev, int id);
#endif