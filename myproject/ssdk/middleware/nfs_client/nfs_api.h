/*
 * nfs_api.h
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: nfs client api.
 *
 **/

#ifndef __NFS_API_H__
#define __NFS_API_H__

#include "nfs.h"

/* attribute */
#define O_RDONLY        00000001
#define O_WRONLY        00000002
#define O_RDWR          00000003

#define O_CREAT         00000100        /* not fcntl */
#define O_EXCL          00000200        /* not fcntl */

#define O_TRUNC         00001000        /* not fcntl */
#define O_APPEND        00002000

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define ENOTBLK         15      /* Block device required */
#define EBUSY           16      /* Device or resource busy */
#define EEXIST          17      /* File exists */
#define EXDEV           18      /* Cross-device link */
#define ENODEV          19      /* No such device */
#define ENOTDIR         20      /* Not a directory */
#define EISDIR          21      /* Is a directory */
#define EINVAL          22      /* Invalid argument */
#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */
#define ENOTTY          25      /* Not a typewriter */
#define ETXTBSY         26      /* Text file busy */
#define EFBIG           27      /* File too large */
#define ENOSPC          28      /* No space left on device */
#define ESPIPE          29      /* Illegal seek */
#define EROFS           30      /* Read-only file system */
#define EMLINK          31      /* Too many links */
#define EPIPE           32      /* Broken pipe */
#define EDOM            33      /* Math argument out of domain of func */
#define ERANGE          34      /* Math result not representable */

/* API */
int nfs_open(nfs_file *file, char *pathname, int flags);
int nfs_close(nfs_file *file);
int nfs_read(nfs_file *file, void *buf, size_t count);
int nfs_write(nfs_file *file, void *buf, size_t count);
#endif
