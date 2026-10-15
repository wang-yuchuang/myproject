#ifndef __ICCMSG_LL_H__
#define __ICCMSG_LL_H__

#include <rpch_common.h>
#include "iccmsg_dpr.h"

#define RPMSG_DEVICES_DIR           "/sys/bus/rpmsg/devices/"
#define DEFAULT_LOCAL_EPT_NAME      "iccmsg_default"


// 定义RPMSG节点类型
typedef enum {
    RPMSG_TYPE_UNKNOWN,  // 不区分类型，匹配所有
    RPMSG_TYPE_IPCC,     // 匹配ipcc类型（名称含soc:ipcc@）
    RPMSG_TYPE_VIRTIO    // 匹配virtio类型（名称含virtio）
} rpmsg_type_t;


typedef struct {
    int fd_ipcc;
    int charfd_ipcc;
    int fd_virtio;
    int charfd_virtio;

    int remote_ept;
    int local_ept;

    char *remote_ipcc_node;
    char *remote_virtio_node;

    char ipcc_dev_path[32];
    char virtio_dev_path[32];
    struct rpmsg_endpoint_info local_ept_info;

}iccmsg_ll_t;





int iccmsg_find_name(const char *name);
int iccmsg_ll_init(iccmsg_ll_t *iccmsg_ll, int remote_ept, char *app_name);
int iccmsg_ll_deinit(iccmsg_ll_t *iccmsg_ll);


#endif
