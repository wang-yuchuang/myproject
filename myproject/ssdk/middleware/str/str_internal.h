#ifndef _STR_INTERNAL_H_
#define _STR_INTERNAL_H_
#include <rpmsg_service.h>
#include <rpmsg_channel.h>

#define ECCCTL_OFF_IDX          6
#ifndef CACHE_LINE
#define CACHE_LINE              CONFIG_ARCH_CACHE_LINE
#endif

#define MAX_BUFF_SIZE_STR       (256)
#define SEC_SAVE_DDR_INFO       (240)

enum state {
    IDLE_STAT,
    INIT_EMMC,
    EMMC_WRITE,
};

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

struct str_info_t {
    uint32_t freq;
    uint32_t reserved[8];
    uint32_t save_csr_index;
    uint32_t restore_csr_index;
    uint32_t save_c_index;
    uint32_t save_i_index;
    uint32_t ddrc_csr[100];
    uint32_t phy_csr[970];
} __PACKED;

static inline int sms_send_msg(rpmsg_channel_t chan, char *buf, int len)
{
    int slen = 0;
    int pos = 0;
    int ret = 0;

    while (pos < len) {
        if (MAX_BUFF_SIZE_STR + pos < len)
            slen = MAX_BUFF_SIZE_STR;
        else
            slen = len - pos;
        ret = rpmsg_channel_send(chan, SEC_SAVE_DDR_INFO, (buf + pos), slen, 500);
        if (ret < 0)
            return ret;
        pos += slen;
    }
    return ret;
}

static inline bool is_need_update(const struct str_info_t *old,
                           const struct str_info_t *new)
{
    if (old->ddrc_csr[0] == 0xffffffff || old->ddrc_csr[0] == 0)
        return true;

    if (old->freq != new->freq)
        return true;

    if (old->ddrc_csr[ECCCTL_OFF_IDX] != new->ddrc_csr[ECCCTL_OFF_IDX])
        return true;

    return false;
}

#endif
