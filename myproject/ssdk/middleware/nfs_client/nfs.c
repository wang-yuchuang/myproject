/*
 * NFS support driver - based on etherboot and U-BOOT's tftp.c
 *
 * Masami Komiya <mkomiya@sonare.it> 2004
 *
 */

/* NOTE: the NFS code is heavily inspired by the NetBSD netboot code (read:
 * large portions are copied verbatim) as distributed in OSKit 0.97.  A few
 * changes were necessary to adapt the code to Etherboot and to fix several
 * inconsistencies.  Also the RPC message preparation is done "by hand" to
 * avoid adding netsprintf() which I find hard to understand and use.  */

/* NOTE 2: Etherboot does not care about things beyond the kernel image, so
 * it loads the kernel image off the boot server (ARP_SERVER) and does not
 * access the client root disk (root-path in dhcpd.conf), which would use
 * ARP_ROOTSERVER.  The root disk is something the operating system we are
 * about to load needs to use.	This is different from the OSKit 0.97 logic.  */

/* NOTE 3: Symlink handling introduced by Anselm M Hoffmeister, 2003-July-14
 * If a symlink is encountered, it is followed as far as possible (recursion
 * possible, maximum 16 steps). There is no clearing of ".."'s inside the
 * path, so please DON'T DO THAT. thx. */

/* NOTE 4: NFSv3 support added by Guillaume GARDET, 2016-June-20.
 * NFSv2 is still used by default. But if server does not support NFSv2, then
 * NFSv3 is used, if available on NFS server. */

//#include <lib/console.h>
//#include <irq.h>
//#include <thread.h>
//#include <platform/interrupts.h>
//#include <heap.h>
#include <string.h>
#include <stdlib.h>
#include "nfs.h"
#include "nfs_api.h"

#include <lwip/api.h>
#include <lwip/sockets.h>
#include <lwip/err.h>
#include <lwip/sys.h>

typedef unsigned char uchar;

#define NFS_RCV_TIMEO 1000

struct nfs_net nfs_net_dev = {
    .rpc_id = 0,
};

/* net src port */
static int get_port(nfs_file *file)
{
    struct nfs_net *nfs_dev = file->private_data;
    nfs_port *nfs_src_port = nfs_dev->nfs_src_port;

    for (int i = 0; i < PORT_NUM; i++) {
        if (nfs_src_port[i].used == 0) {
            nfs_src_port[i].used = 1;
            nfs_src_port[i].port = PORT_BASE + i;
            return nfs_src_port[i].port;
        }
    }

    return -1;
}

/* free net src port */
static int free_port(nfs_file *file, int port)
{
    struct nfs_net *nfs_dev = file->private_data;
    nfs_port *nfs_src_port = nfs_dev->nfs_src_port;
    nfs_src_port[port - PORT_BASE].used = 0;
    nfs_src_port[port - PORT_BASE].port = 0;
    return 0;
}

int lwip_nfs_init(nfs_file *file)
{
    int sock_fd;                        /* local socked */
    struct sockaddr_in client_addr;     /* local addr */
    int ret = 0;
    struct timeval timeout;
    timeout.tv_sec = NFS_RCV_TIMEO / 1000;
    timeout.tv_usec = (NFS_RCV_TIMEO % 1000) * 1000;

    /* creating a Socket Interface */
    sock_fd = lwip_socket(AF_INET, SOCK_DGRAM, 0);

    if (sock_fd == -1) {
        printf("failed to create sock_fd!\n");
        return -1;
    }

    /* socket bind */
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(file->local_ip);
    client_addr.sin_port = htons(file->src_port);
    ret = lwip_bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));

    if (ret < 0) {
        printf("failed to bind sock_fd!\n");
        goto socket_error;

    }

    /* set timeout period for receiving packets */
    ret = lwip_setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                          sizeof(timeout));

    if (ret < 0) {
        printf("failed to setsockopt sock_fd!\n");
        goto socket_error;
    }

    return sock_fd;
socket_error:
    lwip_close(sock_fd);
error:
    return -1;
}

void lwip_nfs_exit(int sock_fd)
{
    lwip_close(sock_fd);
}

static int lwip_nfs_send(int sock_fd, char *server_ip, int port, char *buf,
                         int len)
{
    struct sockaddr_in server_addr;
    int rlen = 0;

    /* config target ip & port */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    rlen = lwip_sendto(sock_fd, (char *)buf, len, 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr));
    return rlen;
}

static int lwip_nfs_recv(int sock_fd, char *server_ip, int port, char *buf,
                         int len)
{
    struct sockaddr_in server_addr;
    socklen_t fromlen;
    int rlen = 0;

    memset(&server_addr, 0, sizeof(server_addr));
    rlen = lwip_recvfrom(sock_fd, (char *)buf, len, 0,
                         (struct sockaddr *)&server_addr, &fromlen);
    return rlen;
}

/* Send and read merged into atomic operations */
static int rpc_net_trans(nfs_file *file, unsigned datalen)
{
    struct nfs_net *nfs_dev = file->private_data;
    struct rpc_t *rpc_pkt = &file->rpc_pkt;
    uint32_t *p;
    int pktlen;
    int rlen;

    /* RPC message size */
    p = rpc_pkt->u.call.data;
    pktlen = (char *)p + datalen * sizeof(uint32_t) - (char *)rpc_pkt;

 //   mutex_acquire_timeout(&nfs_dev->nfs_net_mutex, INFINITE_TIME);
    osMutexAcquire(nfs_dev->nfs_net_mutex, 0);
    /* send Message */
    rlen = lwip_nfs_send(file->sock_fd, file->server_ip, file->currect_port,
                         (char *)&rpc_pkt->u.data[0], pktlen);

    if (rlen != pktlen) {
        printf("lwip_nfs_send len error,pktlen:%d rlen:%d\n", pktlen, rlen);
        return -1;
    }

    /* recv Message */
    memset(rpc_pkt, 0, sizeof(struct rpc_t));
    pktlen = lwip_nfs_recv(file->sock_fd, file->server_ip, file->currect_port,
                           (char *)&rpc_pkt->u.data[0], sizeof(struct rpc_t));

    if (pktlen < 0) {
        printf("lwip_nfs_recv error\n");
        return -1;
    }

    osMutexRelease(nfs_dev->nfs_net_mutex);
    return pktlen;
}
/**************************************************************************
RPC_ADD_CREDENTIALS - Add RPC authentication/verifier entries
**************************************************************************/
static uint32_t *rpc_add_credentials(uint32_t *p)
{
    /* Here's the executive summary on authentication requirements of the
     * various NFS server implementations:  Linux accepts both AUTH_NONE
     * and AUTH_UNIX authentication (also accepts an empty hostname field
     * in the AUTH_UNIX scheme).  *BSD refuses AUTH_NONE, but accepts
     * AUTH_UNIX (also accepts an empty hostname field in the AUTH_UNIX
     * scheme).  To be safe, use AUTH_UNIX and pass the hostname if we have
     * it (if the BOOTP/DHCP reply didn't give one, just use an empty
     * hostname).  */

    /* Provide an AUTH_UNIX credential.  */
    *p++ = htonl(1);        /* AUTH_UNIX */
    *p++ = htonl(20);       /* auth length */
    *p++ = 0;           /* stamp */
    *p++ = 0;           /* hostname string */
    *p++ = 0;           /* uid */
    *p++ = 0;           /* gid */
    *p++ = 0;           /* auxiliary gid list */

    /* Provide an AUTH_NONE verifier.  */
    *p++ = 0;           /* AUTH_NONE */
    *p++ = 0;           /* auth length */

    return p;
}

/* create file properties */
static uint32_t *rpc_add_attributes(uint32_t *p)
{
    *p++ = htonl(CREATE_MODE);      /* mode */
    *p++ = htonl(0xffffffff);       /* uid */
    *p++ = htonl(0xffffffff);       /* gid */
    *p++ = htonl(0xffffffff);       /* size */
    *p++ = htonl(0xffffffff);       /* atime */
    *p++ = htonl(0xffffffff);
    *p++ = htonl(0xffffffff);       /* mtime */
    *p++ = htonl(0xffffffff);

    return p;
}

/* set file properties */
static uint32_t *rpc_add_attr_attributes(uint32_t *p, uint32_t mtime_sec,
        uint32_t mtime_ms)
{
    *p++ = htonl(0xffffffff);       /* mode */
    *p++ = htonl(0xffffffff);       /* uid */
    *p++ = htonl(0xffffffff);       /* gid */
    *p++ = htonl(0x0);              /* size */
    *p++ = htonl(0xffffffff);       /* atime */
    *p++ = htonl(0xffffffff);
    *p++ = htonl(mtime_sec);        /* mtime */
    *p++ = htonl(mtime_ms);
    return p;
}

/**************************************************************************
config RPC Message header
**************************************************************************/
static int rpc_req(nfs_file *file, int rpc_prog, int rpc_proc)
{
    struct nfs_net *nfs_dev = file->private_data;
    struct rpc_t *rpc_pkt = &file->rpc_pkt;
    unsigned long id;

    /* config RPC Message header */
    id = ++nfs_dev->rpc_id;
    file->rpc_id = id;
    rpc_pkt->u.call.id = htonl(id);
    rpc_pkt->u.call.type = htonl(MSG_CALL);
    rpc_pkt->u.call.rpcvers = htonl(2); /* use RPC version 2 */
    rpc_pkt->u.call.prog = htonl(rpc_prog);

    switch (rpc_prog) {
        case PROG_NFS:
            if (file->supported_nfs_versions & NFSV2_FLAG)
                rpc_pkt->u.call.vers = htonl(2);    /* NFS v2 */
            else /* NFSV3_FLAG */
                rpc_pkt->u.call.vers = htonl(3);    /* NFS v3 */

            break;

        case PROG_PORTMAP:
        case PROG_MOUNT:
        default:
            rpc_pkt->u.call.vers = htonl(2);    /* portmapper is version 2 */
    }

    rpc_pkt->u.call.proc = htonl(rpc_proc);

    /* config target port */
    if (rpc_prog == PROG_PORTMAP)
        file->currect_port = file->portmap_port;
    else if (rpc_prog == PROG_MOUNT)
        file->currect_port = file->mount_port;
    else
        file->currect_port = file->nfs_port;

    return 0;
}

/**************************************************************************
RPC_LOOKUP - Lookup RPC Port numbers
**************************************************************************/
static int rpc_lookup_req(nfs_file *file, int prog, int ver)
{
    uint32_t *data = NULL;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;

    data[0] = 0;
    data[1] = 0;   /* auth credential */
    data[2] = 0;
    data[3] = 0;   /* auth verifier */
    data[4] = htonl(prog);
    data[5] = htonl(ver);
    data[6] = htonl(17);        /* IP_UDP */
    data[7] = 0;
    rpc_req(file, PROG_PORTMAP, PORTMAP_GETPORT);
    len = 8;
    return len;
}

/**************************************************************************
Handlers for the rpc lookup reply from server
**************************************************************************/
static int rpc_lookup_reply(nfs_file *file, int prog, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;

    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus)
        return -NFS_RPC_ERR;

    /* get Port info */
    switch (prog) {
        case PROG_MOUNT:
            file->mount_port = ntohl(rpc_pkt->u.reply.data[0]);
            break;

        case PROG_NFS:
            file->nfs_port = ntohl(rpc_pkt->u.reply.data[0]);
            break;
    }

    return 0;
}

int rpc_lookup(nfs_file *file, int prog, int ver)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = rpc_lookup_req(file, prog, ver);

    if (datalen < 0) {
        printf("rpc_lookup_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = rpc_lookup_reply(file, prog, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("rpc_lookup_reply error\n");
        return ret;
    }

    return ret;
}

/**************************************************************************
NFS_MOUNT - Mount an NFS Filesystem
**************************************************************************/
static int nfs_mount_req(nfs_file *file, char *path)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int pathlen = 0;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;

    pathlen = strlen(path);

    p = &(data[0]);
    p = rpc_add_credentials(p);

    *p++ = htonl(pathlen);

    if (pathlen & 3)
        *(p + pathlen / 4) = 0;

    memcpy(p, path, pathlen);
    p += (pathlen + 3) / 4;

    len = (uint32_t *)p - (uint32_t *) & (data[0]);
    rpc_req(file, PROG_MOUNT, MOUNT_ADDENTRY);
    return len;
}

/**************************************************************************
Handlers for the nfs mount reply from server
**************************************************************************/
static int nfs_mount_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;

    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0])
        return -NFS_RPC_ERR;

    /* get Path info:dirfh */
    file->fs_mounted = 1;
    /*  NFSv2 and NFSv3 use same structure */
    memcpy(file->dirfh, rpc_pkt->u.reply.data + 1, NFS_FHSIZE);
    return 0;
}

int nfs_mount(nfs_file *file, char *path)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_mount_req(file, path);

    if (datalen < 0) {
        printf("nfs_mount_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_mount_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_mount_reply error\n");
        return ret;
    }

    return ret;
}

/**************************************************************************
NFS_UMOUNTALL - Unmount all our NFS Filesystems on the Server
**************************************************************************/
static int nfs_umount_req(nfs_file *file, char *path)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int pathlen = 0;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;

    if ((file->mount_port == -1) || (!file->fs_mounted))
        /* Nothing mounted, nothing to umount */
        return 0;

    pathlen = strlen(path);

    p = &(data[0]);
    p = rpc_add_credentials(p);

    *p++ = htonl(pathlen);

    if (pathlen & 3)
        *(p + pathlen / 4) = 0;

    memcpy(p, path, pathlen);
    p += (pathlen + 3) / 4;

    len = (uint32_t *)p - (uint32_t *) & (data[0]);

    rpc_req(file, PROG_MOUNT, MOUNT_UMOUNT);
    return len;
}

/**************************************************************************
Handlers for the nfs umount reply from server
**************************************************************************/
static int nfs_umount_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;

    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus)
        return -NFS_RPC_ERR;

    /* clear Path info:dirfh */
    file->fs_mounted = 0;
    memset(file->dirfh, 0, sizeof(file->dirfh));
    return 0;
}

int nfs_umount(nfs_file *file, char *path)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_umount_req(file, path);

    if (datalen < 0) {
        printf("nfs_umount_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_umount_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_umount_reply error\n");
        return ret;
    }

    return ret;
}

/**************************************************************************
NFS_LOOKUP - Lookup Pathname
**************************************************************************/
static int nfs_lookup_req(nfs_file *file, char *fname)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int fnamelen = 0;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;

    fnamelen = strlen(fname);

    p = &(data[0]);
    p = rpc_add_credentials(p);

    if (file->supported_nfs_versions & NFSV2_FLAG) {
        /* path:dirfh */
        memcpy(p, file->dirfh, NFS_FHSIZE);
        p += (NFS_FHSIZE / 4);
        /* File length & data */
        *p++ = htonl(fnamelen);

        if (fnamelen & 3)
            *(p + fnamelen / 4) = 0;

        memcpy(p, fname, fnamelen);
        p += (fnamelen + 3) / 4;

        len = (uint32_t *)p - (uint32_t *) & (data[0]);

        rpc_req(file, PROG_NFS, NFS_LOOKUP);
    }
    else {    /* NFSV3_FLAG */
        *p++ = htonl(NFS_FHSIZE);   /* Dir handle length */
        memcpy(p, file->dirfh, NFS_FHSIZE);
        p += (NFS_FHSIZE / 4);
        *p++ = htonl(fnamelen);

        if (fnamelen & 3)
            *(p + fnamelen / 4) = 0;

        memcpy(p, fname, fnamelen);
        p += (fnamelen + 3) / 4;

        len = (uint32_t *)p - (uint32_t *) & (data[0]);

        rpc_req(file, PROG_NFS, NFS3PROC_LOOKUP);
    }

    return len;
}

/**************************************************************************
Handlers for the nfs lookup reply from server
**************************************************************************/
static int nfs_lookup_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;
    unsigned len_tmp;

    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0]) {
        switch (ntohl(rpc_pkt->u.reply.astatus)) {
            case NFS_RPC_SUCCESS: /* Not an error */
                if (ntohl(rpc_pkt->u.reply.data[0]) == NFSERR_NOENT) {
                    return -NFS_RPC_NOENT_ERR;
                }

                break;

            case NFS_RPC_PROG_MISMATCH:

                /* Remote can't support NFS version */
                switch (ntohl(rpc_pkt->u.reply.data[0])) {
                    /* Minimal supported NFS version */
                    case 3:

                    /* Full NFS3 versions are not supported */
                    case 4:
                    default:
                        printf("*** ERROR: NFS version not supported");
                        printf(": Requested: V%d, accepted: min V%d - max V%d\n",
                               (file->supported_nfs_versions & NFSV2_FLAG) ?
                               2 : 3,
                               ntohl(rpc_pkt->u.reply.data[0]),
                               ntohl(rpc_pkt->u.reply.data[1]));
                        printf("\n");
                        return -NFS_RPC_V_ERR;
                }

                break;

            case NFS_RPC_PROG_UNAVAIL:
            case NFS_RPC_PROC_UNAVAIL:
            case NFS_RPC_GARBAGE_ARGS:
            case NFS_RPC_SYSTEM_ERR:
            default: /* Unknown error on 'accept state' flag */
                printf("*** ERROR: accept state error (%d)\n",
                       ntohl(rpc_pkt->u.reply.astatus));
                break;
        }

        return -NFS_RPC_ERR;
    }


    /* get file info:filefh */
    if (file->supported_nfs_versions & NFSV2_FLAG) {
        if (((uchar *) & (rpc_pkt->u.reply.data[0]) - (uchar *)(
                    rpc_pkt) + NFS_FHSIZE) > len)
            return -NFS_RPC_ERR;

        memcpy(file->filefh, rpc_pkt->u.reply.data + 1, NFS_FHSIZE);
    }
    else {    /* NFSV3_FLAG */
        file->filefh3_length = ntohl(rpc_pkt->u.reply.data[1]);

        if (file->filefh3_length > NFS3_FHSIZE)
            file->filefh3_length  = NFS3_FHSIZE;

        len_tmp = ((uchar *) & (rpc_pkt->u.reply.data[0]) - (uchar *)(
                       rpc_pkt) + file->filefh3_length);

        if (len_tmp > len)
            return -NFS_RPC_ERR;

        memcpy(file->filefh, rpc_pkt->u.reply.data + 2, file->filefh3_length);
    }

    return 0;
}

int nfs_lookup(nfs_file *file, char *fname)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_lookup_req(file, fname);

    if (datalen < 0) {
        printf("nfs_lookup_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_lookup_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_lookup_reply error\n");
        return ret;
    }

    return ret;
}

/**************************************************************************
NFS_CREATE - Create file
**************************************************************************/
static int nfs_create_req(nfs_file *file, char *fname)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int fnamelen = 0;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;

    fnamelen = strlen(fname);

    p = &(data[0]);
    p = rpc_add_credentials(p);

    /* path:dirfh */
    memcpy(p, file->dirfh, NFS_FHSIZE);
    p += (NFS_FHSIZE / 4);
    /* File length & data */
    *p++ = htonl(fnamelen);

    if (fnamelen & 3)
        *(p + fnamelen / 4) = 0;

    memcpy(p, fname, fnamelen);
    p += (fnamelen + 3) / 4;
    /* File attributes */
    p = rpc_add_attributes(p);

    len = (uint32_t *)p - (uint32_t *) & (data[0]);

    rpc_req(file, PROG_NFS, NFS_CREATE);
    return len;
}

/**************************************************************************
Handlers for the nfs create reply from server
**************************************************************************/
static int nfs_create_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;
    unsigned len_tmp;

    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0])
        return -NFS_RPC_ERR;

    /* get file info:filefh */
    if (file->supported_nfs_versions & NFSV2_FLAG) {
        if (((uchar *) & (rpc_pkt->u.reply.data[0]) - (uchar *)(
                    rpc_pkt) + NFS_FHSIZE) > len)
            return -NFS_RPC_ERR;

        memcpy(file->filefh, rpc_pkt->u.reply.data + 1, NFS_FHSIZE);
    }
    else {    /* NFSV3_FLAG */
        file->filefh3_length = ntohl(rpc_pkt->u.reply.data[1]);

        if (file->filefh3_length > NFS3_FHSIZE)
            file->filefh3_length  = NFS3_FHSIZE;

        len_tmp = ((uchar *) & (rpc_pkt->u.reply.data[0]) - (uchar *)(
                       rpc_pkt) + file->filefh3_length);

        if (len_tmp > len)
            return -NFS_RPC_ERR;

        memcpy(file->filefh, rpc_pkt->u.reply.data + 2, file->filefh3_length);
    }

    return 0;
}

int nfs_create(nfs_file *file, char *fname)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_create_req(file, fname);

    if (datalen < 0) {
        printf("nfs_create_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_create_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_create_reply error\n");
        return ret;
    }

    return ret;
}

static int nfs_getattr_req(nfs_file *file)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;
    p = &(data[0]);
    p = rpc_add_credentials(p);

    /* file:filefh */
    memcpy(p, file->filefh, NFS_FHSIZE);
    p += (NFS_FHSIZE / 4);

    len = (uint32_t *)p - (uint32_t *) & (data[0]);

    rpc_req(file, PROG_NFS, NFS_GETATTR);
    return len;
}

static int nfs_getattr_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;
    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0])
        return -NFS_RPC_ERR;

    /* get file info:size */
    file->file_size = ntohl(rpc_pkt->u.reply.data[6]);

    /* get file info:time */
    file->mtime_sec = ntohl(rpc_pkt->u.reply.data[14]);
    file->mtime_ms = ntohl(rpc_pkt->u.reply.data[15]);
    return 0;
}

int nfs_getattr(nfs_file *file)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_getattr_req(file);

    if (datalen < 0) {
        printf("nfs_getattr_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_getattr_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_getattr_reply error\n");
        return ret;
    }

    return ret;
}

static int nfs_setattr_req(nfs_file *file)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;
    p = &(data[0]);
    p = rpc_add_credentials(p);

    /* file:filefh */
    memcpy(p, file->filefh, NFS_FHSIZE);
    p += (NFS_FHSIZE / 4);

    /* File attributes, size & mtime */
    file->mtime_sec += 20;
    file->mtime_ms = TIME_CONVER;
    p = rpc_add_attr_attributes(p, file->mtime_sec, file->mtime_ms);

    len = (uint32_t *)p - (uint32_t *) & (data[0]);

    rpc_req(file, PROG_NFS, NFS_SETATTR);
    return len;
}

static int nfs_setattr_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;
    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0])
        return -NFS_RPC_ERR;

    return 0;
}

int nfs_setattr(nfs_file *file)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_setattr_req(file);

    if (datalen < 0) {
        printf("nfs_setattr_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_setattr_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_setattr_reply error\n");
        return ret;
    }

    return ret;
}

/**************************************************************************
NFS_READ - Read File on NFS Server
**************************************************************************/
static int nfs_read_req(nfs_file *file, int offset, int readlen)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;
    p = &(data[0]);
    p = rpc_add_credentials(p);

    if (file->supported_nfs_versions & NFSV2_FLAG) {
        /* file:filefh */
        memcpy(p, file->filefh, NFS_FHSIZE);
        p += (NFS_FHSIZE / 4);
        *p++ = htonl(offset);       /* file offset */
        *p++ = htonl(readlen);      /* Read buffer size */
        *p++ = 0;
    }
    else {   /* NFSV3_FLAG */
        *p++ = htonl(file->filefh3_length);
        memcpy(p, file->filefh, file->filefh3_length);
        p += (file->filefh3_length / 4);
        *p++ = htonl(0); /* offset is 64-bit long, so fill with 0 */
        *p++ = htonl(offset);
        *p++ = htonl(readlen);
        *p++ = 0;
    }

    len = (uint32_t *)p - (uint32_t *) & (data[0]);

    rpc_req(file, PROG_NFS, NFS_READ);
    return len;
}

static int nfs3_get_attributes_offset(uint32_t *data)
{
    if (data[1]) {
        /* 'attributes_follow' flag is TRUE,
         * so we have attributes on 21 dwords */
        /* Skip unused values :
            type;   32 bits value,
            mode;   32 bits value,
            nlink;  32 bits value,
            uid;    32 bits value,
            gid;    32 bits value,
            size;   64 bits value,
            used;   64 bits value,
            rdev;   64 bits value,
            fsid;   64 bits value,
            fileid; 64 bits value,
            atime;  64 bits value,
            mtime;  64 bits value,
            ctime;  64 bits value,
        */
        return 22;
    }
    else {
        /* 'attributes_follow' flag is FALSE,
         * so we don't have any attributes */
        return 1;
    }
}

static int nfs_read_reply(nfs_file *file, uchar *pkt, unsigned len, void *buf)
{
    struct rpc_t *rpc_pkt;
    int rlen = 0;
    uchar *data_ptr;
    unsigned len_tmp;

    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0]) {
        if (rpc_pkt->u.reply.rstatus)
            return -NFS_RPC_ERR;

        if (rpc_pkt->u.reply.astatus)
            return -NFS_RPC_ERR;

        return -ntohl(rpc_pkt->u.reply.data[0]);
    }

    /* Parsing packet */
    if (file->supported_nfs_versions & NFSV2_FLAG) {
        rlen = ntohl(rpc_pkt->u.reply.data[18]);
        data_ptr = (uchar *) & (rpc_pkt->u.reply.data[19]);
    }
    else {    /* NFSV3_FLAG */
        int nfsv3_data_offset =
            nfs3_get_attributes_offset(rpc_pkt->u.reply.data);

        /* count value */
        rlen = ntohl(rpc_pkt->u.reply.data[1 + nfsv3_data_offset]);
        /* Skip unused values :
            EOF:        32 bits value,
            data_size:  32 bits value,
        */
        data_ptr = (uchar *)
                   & (rpc_pkt->u.reply.data[4 + nfsv3_data_offset]);
    }

    len_tmp = ((uchar *) & (rpc_pkt->u.reply.data[0]) - (uchar *)(rpc_pkt) + rlen);

    if (len_tmp > len)
        return -NFS_RPC_ERR;


    memcpy(buf, data_ptr, rlen);
    return rlen;
}

int nfs_rpc_read(nfs_file *file, void *buf, int file_offset, int len)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_read_req(file, file_offset, len);

    if (datalen < 0) {
        printf("nfs_setattr_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_read_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen, buf);

    if (ret < 0) {
        printf("nfs_read_reply error\n");
        return ret;
    }

    return ret;
}

/**************************************************************************
NFS_WRITE - Write File on NFS Server
**************************************************************************/
static int nfs_write_req(nfs_file *file, int offset, int writelen, void *buf)
{
    uint32_t *data = NULL;
    uint32_t *p;
    int len = 0;

    memset(&file->rpc_pkt, 0, sizeof(struct rpc_t));
    data = file->rpc_pkt.u.call.data;
    p = &(data[0]);
    p = rpc_add_credentials(p);

    /* file:filefh */
    memcpy(p, file->filefh, NFS_FHSIZE);
    p += (NFS_FHSIZE / 4);
    *p++ = htonl(offset);               /* behin offset */
    *p++ = htonl(offset);               /* offset */
    *p++ = htonl(writelen);             /* total count */
    *p++ = htonl(writelen);             /* length */

    /* file data */
    memcpy(p, buf, writelen);
    p += (writelen + 3) / 4;
    len = (uint32_t *)p - (uint32_t *) & (data[0]);

    rpc_req(file, PROG_NFS, NFS_WRITE);
    return len;
}

static int nfs_write_reply(nfs_file *file, uchar *pkt, unsigned len)
{
    struct rpc_t *rpc_pkt;
    rpc_pkt = (struct rpc_t *)pkt;

    if (ntohl(rpc_pkt->u.reply.id) > file->rpc_id)
        return -NFS_RPC_ERR;
    else if (ntohl(rpc_pkt->u.reply.id) < file->rpc_id)
        return -NFS_RPC_DROP;

    if (rpc_pkt->u.reply.rstatus  ||
            rpc_pkt->u.reply.verifier ||
            rpc_pkt->u.reply.astatus  ||
            rpc_pkt->u.reply.data[0])
        return -NFS_RPC_ERR;

    return len;
}

int nfs_rpc_write(nfs_file *file, void *buf, int file_offset, int len)
{
    int ret = 0;
    int datalen = 0;
    int pktlen;

    datalen = nfs_write_req(file, file_offset, len, buf);

    if (datalen < 0) {
        printf("nfs_write_req error\n");
        return datalen;
    }

    pktlen = rpc_net_trans(file, datalen);

    if (pktlen < 0) {
        printf("rpc_net_trans error\n");
        return pktlen;
    }

    ret = nfs_write_reply(file, (uchar *)&file->rpc_pkt.u.data[0], pktlen);

    if (ret < 0) {
        printf("nfs_write_reply error\n");
        return ret;
    }

    return len;
}

int nfs_file_init(nfs_file *file)
{
    file->private_data = &nfs_net_dev;
    /* Fixed source port */
    file->src_port = get_port(file);

    if (file->src_port < 0) {
        printf("failed to get_port!\n");
        return -EBUSY;
    }

    printf("src_port:%d!\n", file->src_port);
    file->sock_fd = lwip_nfs_init(file);

    if (file->sock_fd < 0) {
        printf("failed to lwip_nfs_init!\n");
        return -EBUSY;
    }

    return 0;
}

int nfs_file_exit(nfs_file *file)
{
    int ret = 0;

    if (file->sock_fd >= 0)
        lwip_nfs_exit(file->sock_fd);

    if (file->src_port >= 0)
        free_port(file, file->src_port);

    file->private_data = NULL;
    return ret;
}

int nfs_state(void)
{
    struct nfs_net *nfs_dev = &nfs_net_dev;
    nfs_dev->rpc_id = 0;
    memset(nfs_dev->nfs_src_port, 0, sizeof(nfs_dev->nfs_src_port));
    nfs_dev->nfs_net_mutex = osMutexNew(NULL);
    return 0;
}

//APP_START(nfs_state)
//.entry = (app_entry)nfs_state,
//APP_END

