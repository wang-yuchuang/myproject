/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 */

#ifndef __NFS_H__
#define __NFS_H__

#include <cmsis_os2.h>

/* PORTMAP UDP port */
#define SUNRPC_PORT     111

/* PROG type */
#define PROG_PORTMAP    100000
#define PROG_NFS        100003
#define PROG_MOUNT      100005

/* MSG type */
#define MSG_CALL        0
#define MSG_REPLY       1

/* PORTMAP type */
#define PORTMAP_GETPORT 3

/* MOUNT type */
#define MOUNT_ADDENTRY  1
#define MOUNT_UMOUNT    3
#define MOUNT_UMOUNTALL 4

/* NFS type */
#define NFS_GETATTR     1
#define NFS_SETATTR     2
#define NFS_LOOKUP      4
#define NFS_CREATE      9
#define NFS_READLINK    5
#define NFS_READ        6
#define NFS_WRITE       8

#define NFS3PROC_LOOKUP 3

#define NFS_FHSIZE      32
#define NFS3_FHSIZE     64

#define NFSERR_PERM     1
#define NFSERR_NOENT    2
#define NFSERR_ACCES    13
#define NFSERR_ISDIR    21
#define NFSERR_INVAL    22

#define CREATE_MODE     0x81a4
#define TIME_CONVER     (1000 * 1000)

/* local ERR info */
#define NFS_RPC_ERR         1
#define NFS_RPC_DROP        124
#define NFS_RPC_ID_ERR      125
#define NFS_RPC_UMOUNT      126
#define NFS_RPC_V_ERR       127
#define NFS_RPC_MEM_ERR     128
#define NFS_RPC_NOENT_ERR   129

/*
 * Block size used for NFS read accesses.  A RPC reply packet (including  all
 * headers) must fit within a single Ethernet frame to avoid fragmentation.
 * However, if CONFIG_IP_DEFRAG is set, a bigger value could be used.  In any
 * case, most NFS servers are optimized for a power of 2.
 */
#define NFS_READ_SIZE   2048    /* biggest power of two that fits Ether frame */
#define NFS_WRITE_SIZE  2048    /* biggest power of two that fits Ether frame */
#define NFS_MAX_ATTRS   26
#define HASHES_PER_LINE 65  /* Number of "loading" hashes per line  */

#define NFSV2_FLAG 1
#define NFSV3_FLAG 1 << 1

/* nfs src port */
#define PORT_NUM    128
#define PORT_BASE   1000

/* nfs file */
#define IP_SIZE     128
#define FILE_SIZE   1024

/* Values for Accept State flag on RPC answers (See: rfc1831) */
enum rpc_accept_stat {
    NFS_RPC_SUCCESS = 0,    /* RPC executed successfully */
    NFS_RPC_PROG_UNAVAIL = 1,   /* remote hasn't exported program */
    NFS_RPC_PROG_MISMATCH = 2,  /* remote can't support version # */
    NFS_RPC_PROC_UNAVAIL = 3,   /* program can't support procedure */
    NFS_RPC_GARBAGE_ARGS = 4,   /* procedure can't decode params */
    NFS_RPC_SYSTEM_ERR = 5, /* errors like memory allocation failure */
};

/* RPC message format */
struct rpc_t {
    union {
        uint8_t data[NFS_READ_SIZE + (6 + NFS_MAX_ATTRS) *
                                   sizeof(uint32_t)];
        struct {
            uint32_t id;
            uint32_t type;
            uint32_t rpcvers;
            uint32_t prog;
            uint32_t vers;
            uint32_t proc;
            uint32_t data[1];
        } call;
        struct {
            uint32_t id;
            uint32_t type;
            uint32_t rstatus;
            uint32_t verifier;
            uint32_t v2;
            uint32_t astatus;
            uint32_t data[NFS_READ_SIZE / sizeof(uint32_t) +
                                        NFS_MAX_ATTRS];
        } reply;
    } u;
};

/* nfs src port */
typedef struct {
    int port;
    int used;
} nfs_port;

struct nfs_net {
    unsigned long rpc_id;
    osMutexId_t nfs_net_mutex;
    nfs_port nfs_src_port[PORT_NUM];
};

/* nfs_file */
typedef struct {
    /*net ip*/
    char server_ip[IP_SIZE];
    char local_ip[IP_SIZE];

    /* socket info */
    int sock_fd;                /* client socked */
    int currect_port;
    int portmap_port;
    int nfs_port;
    int mount_port;
    int src_port;

    /* Boot File name */
    char file_name[FILE_SIZE];
    char *nfs_filename;
    char *nfs_path;

    /* file info */
    uint32_t file_offset;       /* file offset */
    uint32_t file_size;         /* file size */

    uint32_t mtime_sec;
    uint32_t mtime_ms;

    /* nfs state info */
    char supported_nfs_versions;
    unsigned long rpc_id;       /* currect message id */
    int fs_mounted;
    int flags;                  /* file flags */
    char dirfh[NFS_FHSIZE];     /* NFSv2 / NFSv3 file handle of directory */
    char filefh[NFS3_FHSIZE];   /* NFSv2 / NFSv3 file handle */
    int filefh3_length;         /* (variable) length of filefh when NFSv3 */

    struct rpc_t rpc_pkt;
    void *private_data;         /* struct nfs_net */
} nfs_file;

/* nfs API */
int nfs_file_init(nfs_file *file);
int nfs_file_exit(nfs_file *file);

int rpc_lookup(nfs_file *file, int prog, int ver);

int nfs_mount(nfs_file *file, char *path);
int nfs_umount(nfs_file *file, char *path);

int nfs_lookup(nfs_file *file, char *fname);
int nfs_create(nfs_file *file, char *fname);

int nfs_getattr(nfs_file *file);
int nfs_setattr(nfs_file *file);

int nfs_rpc_read(nfs_file *file, void *buf, int file_offset, int len);
int nfs_rpc_write(nfs_file *file, void *buf, int file_offset, int len);

/**********************************************************************/

#endif /* __NFS_H__ */

