/*
 * errno.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: errno defines.
 *
 * Revision History:
 * -----------------
 */

#ifndef ERRNO_H
#define ERRNO_H

#include <compiler.h>

__BEGIN_CDECLS

#if defined(__GNUC__)
#include <sys/errno.h>

#else

#if defined(__ICCARM__)
#include <c/errno.h>
#define ERR_BASE        _NERR
#else
#define ERR_BASE        0
#ifndef errno
extern int *ssdk_geterrno(void);
#define errno (*ssdk_geterrno())
#endif
#endif

#define EPERM           (ERR_BASE + 1)
#define ENOENT          (ERR_BASE + 2)
#define ESRCH           (ERR_BASE + 3)
#define EINTR           (ERR_BASE + 4)
#define EIO             (ERR_BASE + 5)
#define ENXIO           (ERR_BASE + 6)
#define E2BIG           (ERR_BASE + 7)
#define ENOEXEC         (ERR_BASE + 8)
#define EBADF           (ERR_BASE + 9)
#define ECHILD          (ERR_BASE + 10)
#define EAGAIN          (ERR_BASE + 11)

#ifndef ENOMEM
#define ENOMEM          (ERR_BASE + 12)
#endif

#define EACCES          (ERR_BASE + 13)
#define EFAULT          (ERR_BASE + 14)
#define ENOTBLK         (ERR_BASE + 15)
#define EBUSY           (ERR_BASE + 16)
#define EEXIST          (ERR_BASE + 17)
#define EXDEV           (ERR_BASE + 18)
#define ENODEV          (ERR_BASE + 19)
#define ENOTDIR         (ERR_BASE + 20)
#define EISDIR          (ERR_BASE + 21)

#ifndef EINVAL
#define EINVAL          (ERR_BASE + 22)
#endif

#define ENFILE          (ERR_BASE + 23)
#define EMFILE          (ERR_BASE + 24)
#define ENOTTY          (ERR_BASE + 25)
#define ETXTBSY         (ERR_BASE + 26)
#define EFBIG           (ERR_BASE + 27)
#define ENOSPC          (ERR_BASE + 28)
#define ESPIPE          (ERR_BASE + 29)
#define EROFS           (ERR_BASE + 30)
#define EMLINK          (ERR_BASE + 31)
#define EPIPE           (ERR_BASE + 32)

#ifndef EDOM
#define EDOM            (ERR_BASE + 33)
#endif

#ifndef ERANGE
#define ERANGE          (ERR_BASE + 34)
#endif

#define EDEADLK         (ERR_BASE + 35)
#define ENAMETOOLONG    (ERR_BASE + 36)
#define ENOLCK          (ERR_BASE + 37)
#define ENOSYS          (ERR_BASE + 38)
#define ENOTEMPTY       (ERR_BASE + 39)
#define ELOOP           (ERR_BASE + 40)
#define EWOULDBLOCK     EAGAIN
#define ENOMSG          (ERR_BASE + 42)
#define EIDRM           (ERR_BASE + 43)
#define ECHRNG          (ERR_BASE + 44)
#define EL2NSYNC        (ERR_BASE + 45)
#define EL3HLT          (ERR_BASE + 46)
#define EL3RST          (ERR_BASE + 47)
#define ELNRNG          (ERR_BASE + 48)
#define EUNATCH         (ERR_BASE + 49)
#define ENOCSI          (ERR_BASE + 50)
#define EL2HLT          (ERR_BASE + 51)
#define EBADE           (ERR_BASE + 52)
#define EBADR           (ERR_BASE + 53)
#define EXFULL          (ERR_BASE + 54)
#define ENOANO          (ERR_BASE + 55)
#define EBADRQC         (ERR_BASE + 56)
#define EBADSLT         (ERR_BASE + 57)
#define EDEADLOCK       EDEADLK
#define EBFONT          (ERR_BASE + 59)
#define ENOSTR          (ERR_BASE + 60)
#define ENODATA         (ERR_BASE + 61)
#define ETIME           (ERR_BASE + 62)
#define ENOSR           (ERR_BASE + 63)
#define ENONET          (ERR_BASE + 64)
#define ENOPKG          (ERR_BASE + 65)
#define EREMOTE         (ERR_BASE + 66)
#define ENOLINK         (ERR_BASE + 67)
#define EADV            (ERR_BASE + 68)
#define ESRMNT          (ERR_BASE + 69)
#define ECOMM           (ERR_BASE + 70)
#define EPROTO          (ERR_BASE + 71)
#define EMULTIHOP       (ERR_BASE + 72)
#define EDOTDOT         (ERR_BASE + 73)
#define EBADMSG         (ERR_BASE + 74)
#define EOVERFLOW       (ERR_BASE + 75)
#define ENOTUNIQ        (ERR_BASE + 76)
#define EBADFD          (ERR_BASE + 77)
#define EREMCHG         (ERR_BASE + 78)
#define ELIBACC         (ERR_BASE + 79)
#define ELIBBAD         (ERR_BASE + 80)
#define ELIBSCN         (ERR_BASE + 81)
#define ELIBMAX         (ERR_BASE + 82)
#define ELIBEXEC        (ERR_BASE + 83)

#ifndef EILSEQ
#define EILSEQ          (ERR_BASE + 84)
#endif

#define ERESTART        (ERR_BASE + 85)
#define ESTRPIPE        (ERR_BASE + 86)
#define EUSERS          (ERR_BASE + 87)
#define ENOTSOCK        (ERR_BASE + 88)
#define EDESTADDRREQ    (ERR_BASE + 89)
#define EMSGSIZE        (ERR_BASE + 90)
#define EPROTOTYPE      (ERR_BASE + 91)
#define ENOPROTOOPT     (ERR_BASE + 92)
#define EPROTONOSUPPORT (ERR_BASE + 93)
#define ESOCKTNOSUPPORT (ERR_BASE + 94)
#define EOPNOTSUPP      (ERR_BASE + 95)
#define ENOTSUP         EOPNOTSUPP
#define EPFNOSUPPORT    (ERR_BASE + 96)
#define EAFNOSUPPORT    (ERR_BASE + 97)
#define EADDRINUSE      (ERR_BASE + 98)
#define EADDRNOTAVAIL   (ERR_BASE + 99)
#define ENETDOWN        (ERR_BASE + 100)
#define ENETUNREACH     (ERR_BASE + 101)
#define ENETRESET       (ERR_BASE + 102)
#define ECONNABORTED    (ERR_BASE + 103)
#define ECONNRESET      (ERR_BASE + 104)
#define ENOBUFS         (ERR_BASE + 105)
#define EISCONN         (ERR_BASE + 106)
#define ENOTCONN        (ERR_BASE + 107)
#define ESHUTDOWN       (ERR_BASE + 108)
#define ETOOMANYREFS    (ERR_BASE + 109)
#define ETIMEDOUT       (ERR_BASE + 110)
#define ECONNREFUSED    (ERR_BASE + 111)
#define EHOSTDOWN       (ERR_BASE + 112)
#define EHOSTUNREACH    (ERR_BASE + 113)
#define EALREADY        (ERR_BASE + 114)
#define EINPROGRESS     (ERR_BASE + 115)
#define ESTALE          (ERR_BASE + 116)
#define EUCLEAN         (ERR_BASE + 117)
#define ENOTNAM         (ERR_BASE + 118)
#define ENAVAIL         (ERR_BASE + 119)
#define EISNAM          (ERR_BASE + 120)
#define EREMOTEIO       (ERR_BASE + 121)
#define EDQUOT          (ERR_BASE + 122)
#define ENOMEDIUM       (ERR_BASE + 123)
#define EMEDIUMTYPE     (ERR_BASE + 124)
#define ECANCELED       (ERR_BASE + 125)
#define ENOKEY          (ERR_BASE + 126)
#define EKEYEXPIRED     (ERR_BASE + 127)
#define EKEYREVOKED     (ERR_BASE + 128)
#define EKEYREJECTED    (ERR_BASE + 129)
#define EOWNERDEAD      (ERR_BASE + 130)
#define ENOTRECOVERABLE (ERR_BASE + 131)
#define ERFKILL         (ERR_BASE + 132)
#define EHWPOISON       (ERR_BASE + 133)
#endif

__END_CDECLS

#endif /* ERRNO_H */