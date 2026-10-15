/*
 * nfs_api.c
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: nfs client api.
 *
 **/

//#include <lib/console.h>
//#include <irq.h>
//#include <thread.h>
//#include <platform/interrupts.h>
//#include <heap.h>
//#include <string.h>
#include <stdlib.h>
//#include <app.h>
#include "nfs_api.h"

#include <lwip/api.h>
#include <lwip/sockets.h>
#include <lwip/err.h>
#include <lwip/sys.h>

/* local host ip */
#define HOST_IP     "172.20.1.12"

static char *basename_nfs(char *path)
{
    char *fname;

    fname = path + strlen(path) - 1;

    while (fname >= path) {
        if (*fname == '/') {
            fname++;
            break;
        }

        fname--;
    }

    return fname;
}

static char *dirname(char *path)
{
    char *fname;

    fname = basename_nfs(path);
    --fname;
    *fname = '\0';
    return path;
}

/* set file attribute:size = 0 */
int nfs_attribute_trunc(nfs_file *file)
{
    int ret = 0;

    ret = nfs_getattr(file);

    if (ret < 0) {
        printf("failed to nfs_getattr %s!\n", file->nfs_filename);
        return -ENOENT;
    }

    ret = nfs_setattr(file);

    if (ret < 0) {
        printf("failed to nfs_setattr %s!\n", file->nfs_filename);
        return -ENOENT;
    }

    return ret;
}

/* set the file index to the end of the file */
int nfs_attribute_append(nfs_file *file)
{
    int ret = 0;

    ret = nfs_getattr(file);

    if (ret < 0) {
        printf("failed to nfs_getattr %s!\n", file->nfs_filename);
        return -ENOENT;
    }

    file->file_offset = file->file_size;
    return ret;
}

/* nfs file open,base on pathname & flags to configuration nfs file */
int nfs_open(nfs_file *file, char *pathname, int flags)
{
    int ret = 0;
    char *colon;

    /* clear nfs file */
    memset(file->server_ip, 0, sizeof(nfs_file));

    /* pathname format:server_ip:/nfs_path/nfs_filename */
    /*  Get the server_ip and file name based on the pathname */
    colon = strchr(pathname, ':');

    if (colon) {
        memcpy(file->server_ip, pathname, colon - pathname);
        memcpy(file->file_name, colon + 1, strlen(colon + 1));
    }
    else {
        ret = -ENXIO;
        goto error;
    }

    /* Get the NFS file and path based on the file name */
    file->nfs_filename = basename_nfs(file->file_name);
    file->nfs_path     = dirname(file->file_name);

    /* file parameter init */
    file->flags = flags;
    file->file_offset = 0;      /* file offset */
    file->supported_nfs_versions = NFSV2_FLAG;
    file->fs_mounted = 0;
    file->portmap_port = SUNRPC_PORT;
    memcpy(file->local_ip, HOST_IP, strlen(HOST_IP));

    /* nfs file init */
    ret = nfs_file_init(file);

    if (ret < 0) {
        printf("failed to nfs_file_init!\n");
        ret = -ENOENT;
        goto file_exit_error;
    }

    /* RPC protocol:portmap PROG_MOUNT */
    if (file->supported_nfs_versions & NFSV2_FLAG)
        ret = rpc_lookup(file, PROG_MOUNT, 1);
    else  /* NFSV3_FLAG */
        ret = rpc_lookup(file, PROG_MOUNT, 3);

    if (ret < 0) {
        printf("failed to rpc_lookup PROG_MOUNT!\n");
        ret = -ENOENT;
        goto file_exit_error;
    }

    /* RPC protocol:portmap PROG_NFS */
    if (file->supported_nfs_versions & NFSV2_FLAG)
        ret = rpc_lookup(file, PROG_NFS, 2);
    else  /* NFSV3_FLAG */
        ret = rpc_lookup(file, PROG_NFS, 3);

    if (ret < 0) {
        printf("failed to rpc_lookup PROG_NFS!\n");
        ret = -ENOENT;
        goto file_exit_error;
    }

    /* RPC protocol:nfs mount */
    ret = nfs_mount(file, file->nfs_path);

    if (ret < 0) {
        printf("failed to nfs_mount %s!\n", file->nfs_path);
        ret = -ENOENT;
        goto nfs_umount_error;
    }

    /* RPC protocol:nfs lookup */
    ret = nfs_lookup(file, file->nfs_filename);

    if (file->flags & O_EXCL) {
        /* O_EXCL:Check whether files are stored */
        if (ret == -NFS_RPC_NOENT_ERR) {
            /* O_CREAT:Create a file */
            if (file->flags & O_CREAT) {
                /* RPC protocol:nfs create */
                ret = nfs_create(file, file->nfs_filename);

                if (ret < 0) {
                    printf("failed to nfs_create %s!\n", file->nfs_filename);
                    ret = -ENOENT;
                    goto nfs_umount_error;
                }
            }
        }
        /* File storage, -EEXIST error reported */
        else {
            printf("%s File exists!\n", file->nfs_filename);
            ret = -EEXIST;
            goto nfs_umount_error;
        }
    }
    else {
        /* O_CREAT:Create a file */
        if ((ret == -NFS_RPC_NOENT_ERR) && (file->flags & O_CREAT)) {
            /* RPC protocol:nfs create */
            ret = nfs_create(file, file->nfs_filename);

            if (ret < 0) {
                printf("failed to nfs_create %s!\n", file->nfs_filename);
                ret = -ENOENT;
                goto nfs_umount_error;
            }
        }
        else if (ret < 0) {
            printf("failed to nfs_lookup %s!\n", file->nfs_filename);
            ret = -ENOENT;
            goto nfs_umount_error;
        }
    }

    /* O_TRUNC:Set the file size to 0 */
    if (file->flags & O_TRUNC) {
        ret = nfs_attribute_trunc(file);

        if (ret < 0)
            goto nfs_umount_error;
    }

    /* O_APPEND:set the file index to the end of the file */
    if (file->flags & O_APPEND) {
        ret = nfs_attribute_append(file);

        if (ret < 0)
            goto nfs_umount_error;
    }

    return ret;

nfs_umount_error:
    nfs_umount(file, file->nfs_path);
file_exit_error:
    nfs_file_exit(file);
error:
    return ret;
}
int nfs_close(nfs_file *file)
{
    int ret = 0;
    /* RPC protocol:nfs umount */
    ret = nfs_umount(file, file->nfs_path);

    if (ret < 0) {
        printf("failed to nfs_umountall %s!\n", file->nfs_path);
        return -ENOENT;
    }

    /* nfs file exit */
    ret = nfs_file_exit(file);

    if (ret < 0) {
        printf("failed to nfs_file_exit!\n");
        return -ENOENT;
    }

    return ret;
}

int nfs_read(nfs_file *file, void *buf, size_t count)
{
    int ret = 0;
    size_t rlen = 0;
    size_t rcount = 0;
    size_t roffset = 0;

    /* O_RDONLY:A read-only file */
    if (!(file->flags & O_RDONLY)) {
        printf("%s write only!\n", file->nfs_filename);
        return -EACCES;
    }

    /* RPC protocol:nfs getattr */
    ret = nfs_getattr(file);

    if (ret < 0) {
        printf("failed to nfs_getattr %s!\n", file->nfs_filename);
        return -ENOENT;
    }

    /* File index to the end of the file */
    if (file->file_size == file->file_offset)
        return 0;
    else if (file->file_size < file->file_offset) {
        return -ENXIO;
    }

    /* Calculates the readable size */
    rcount = count <= (file->file_size - file->file_offset) ? count :
             (file->file_size - file->file_offset);

    while (1) {
        /* Single read size */
        rlen = rcount <= NFS_READ_SIZE ? rcount : NFS_READ_SIZE;
        /* RPC protocol:nfs read */
        ret = nfs_rpc_read(file, buf, file->file_offset, rlen);

        if (ret < 0) {
            printf("failed to nfs_read %s!\n", file->nfs_filename);
            return -ENOENT;
        }

        /* Offset calculation */
        file->file_offset += ret;
        buf += ret;
        roffset += ret;
        rcount -= ret;

        if (rcount <= 0)
            break;
    }

    return roffset;
}
int nfs_write(nfs_file *file, void *buf, size_t count)
{
    int ret = 0;
    size_t rcount = 0;
    size_t rlen = 0;
    size_t roffset = 0;

    /* O_WRONLY:A write-only file */
    if (!(file->flags & O_WRONLY)) {
        printf("%s read only!\n", file->nfs_filename);
        return -EACCES;
    }

    rcount = count;

    while (1) {
        /* Single write size */
        rlen = rcount <= NFS_WRITE_SIZE ? rcount : NFS_WRITE_SIZE;
        /* RPC protocol:nfs write */
        ret = nfs_rpc_write(file, buf, file->file_offset, rlen);

        if (ret < 0) {
            printf("failed to nfs_read %s!\n", file->nfs_filename);
            return -ENOENT;
        }

        /* Offset calculation */
        file->file_offset += ret;
        buf += ret;
        roffset += ret;
        rcount -= ret;

        if (rcount <= 0)
            break;
    }

    return roffset;
}
