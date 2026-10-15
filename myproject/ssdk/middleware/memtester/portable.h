/**
 * @file portable.h
 * @brief memtester port header
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#define mlock(a, b) 0
#define munlock(a, b)
#define exit(a)
#define stat(a,b) (-1)

#undef _SC_VERSION
#undef _SC_PAGE_SIZE

#define MAP_FAILED	((void *)-1)

#define open(...) (1)
#define mmap(addr, length, prot, flags, fd, offset) (offset)

#define fprintf(file, x...) printf(x)

#define fputc(ch, file) printf(ch)
#define fputs(ch, file) printf(ch)
