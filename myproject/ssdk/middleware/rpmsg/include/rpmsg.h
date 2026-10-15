#ifndef RPMSG_H_
#define RPMSG_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <types.h>
#include <cmsis_os2.h>
#include <lib/list.h>
#include <param.h>

#define RPMSG_TYPE_VIRTIO       0
#define RPMSG_TYPE_IPCC         1
#define RPMSG_TYPE_NUM          2

#define RPMSG_SUCCESS     (0)
#define RPMSG_NULL        ((void *)0)
#define RPMSG_REMOTE      (0)
#define RPMSG_MASTER      (1)
#define RPMSG_TRUE        (1)
#define RPMSG_FALSE       (0)
#define RPMSG_ADDR_ANY    (0xFFFFFFFF)
#define RPMSG_RELEASE     (0)
#define RPMSG_HOLD        (1)
#define RPMSG_DONT_BLOCK  (0)
#define RPMSG_BLOCK       (0xFFFFFFFF)

#define RPMSG_ERRORS_BASE      (-5000)
#define RPMSG_ERR_NO_MEM       (RPMSG_ERRORS_BASE - 1)
#define RPMSG_ERR_BUFF_SIZE    (RPMSG_ERRORS_BASE - 2)
#define RPMSG_ERR_PARAM        (RPMSG_ERRORS_BASE - 3)
#define RPMSG_ERR_DEV_ID       (RPMSG_ERRORS_BASE - 4)
#define RPMSG_ERR_MAX_VQ       (RPMSG_ERRORS_BASE - 5)
#define RPMSG_ERR_NO_BUFF      (RPMSG_ERRORS_BASE - 6)
#define RPMSG_NOT_READY        (RPMSG_ERRORS_BASE - 7)
#define RPMSG_ALREADY_DONE     (RPMSG_ERRORS_BASE - 8)

#define RPMSG_LINKSTATE_DOWN        (0)
#define RPMSG_LINKSTATE_SYNC        (1)
#define RPMSG_LINKSTATE_SYNA        (2)
#define RPMSG_LINKSTATE_UP          (3)
#define RPMSG_LINKSTATE_ERRSTATE    (4)

struct rpmsg_endpoint;
struct rpmsg_instance;

typedef int (*rpmsg_rx_cb_t)(void *payload, int payload_len, unsigned long src, void *priv);

struct rpmsg_instance_ops
{
    int (*master_init)(struct rpmsg_instance *ins, void *cfg);
    int (*remote_init)(struct rpmsg_instance *ins, void *cfg);
    int (*deinit)(struct rpmsg_instance *ins);
    int (*is_master)(struct rpmsg_instance *ins);
    int (*get_buffer_num)(struct rpmsg_instance *ins);
    int (*get_payload_size)(struct rpmsg_instance *ins);
    int (*get_link_status)(struct rpmsg_instance *ins);
    int (*set_link_status)(struct rpmsg_instance *ins, int32_t status);
    int (*kick_remote)(struct rpmsg_instance *ins);
};

struct rpmsg_lower_ops
{
    int (*tx)(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx);
    void *(*tx_alloc)(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx);
    void *(*rx)(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx);
    int (*rx_free)(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx);
    int (*tx_direct)(struct rpmsg_instance *ins, unsigned long src, unsigned long dst,
                     char *data, unsigned long len, unsigned long timeout);
};

struct rpmsg_endpoint
{
    unsigned long     addr;        /*!< endpoint address */
    rpmsg_rx_cb_t     rx_cb;       /*!< ISR callback function */
    void              *rx_cb_data; /*!< ISR callback data */
    struct list_node  node;
    void              *priv;       /* store rpmsg_channel_t */
};

struct rpmsg_instance
{
    osMutexId_t                mutex;
    struct list_node           endpoints;
    struct rpmsg_instance_ops  *ins_ops;
    struct rpmsg_lower_ops     *lower_ops;
    uint32_t                   rpmsg_type;
    uint32_t                   remote_proc;
    void                       *priv;
};

int rpmsg_master_init(struct rpmsg_instance *ins, void *cfg);
int rpmsg_remote_init(struct rpmsg_instance *ins, void *cfg);
int rpmsg_deinit(struct rpmsg_instance *ins);
struct rpmsg_endpoint *rpmsg_create_ept(struct rpmsg_instance *ins,
                                        unsigned long addr,
                                        rpmsg_rx_cb_t rx_cb,
                                        void *rx_cb_data);
int rpmsg_destroy_ept(struct rpmsg_instance *ins, struct rpmsg_endpoint *ept);
int rpmsg_send(struct rpmsg_instance *ins,
               struct rpmsg_endpoint *ept,
               unsigned long dst,
               char *data,
               unsigned long size,
               unsigned long timeout);
int rpmsg_send_nocopy(struct rpmsg_instance *ins,
                      struct rpmsg_endpoint *ept,
                      unsigned long dst,
                      void *data,
                      unsigned long size);
void *rpmsg_alloc_tx_buffer(struct rpmsg_instance *ins,
                            unsigned long *size,
                            unsigned long timeout);
int rpmsg_release_rx_buffer(struct rpmsg_instance *ins, void *rxbuf);

int rpmsg_is_link_up(struct rpmsg_instance *ins);

int rpmsg_kick_remote(struct rpmsg_instance *ins);

#if defined(__cplusplus)
}
#endif

#endif /* RPMSG_H_ */
