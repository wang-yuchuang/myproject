/**
 * @file io.c
 * @brief IAR read/write code.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <LowLevelIOInterface.h>

extern char board_getc(void);
extern void board_putc(char c);

size_t __read(int file, unsigned char *ptr, size_t len)
{
    int i;

    /* only support stdin */
    if (file != _LLIO_STDIN)
    {
        return -1;
    }

    for (i = 0; i < len; i++) {
        ptr[i] = board_getc();
        if ((ptr[i] == '\n') || (ptr[i] == '\r'))
        {
            break;
        }
    }

    return i;
}

size_t __write(int file, unsigned char const *ptr, size_t len)
{
    int i;

    /* only support stdout, stdin, stderr */
    if ((file != _LLIO_STDOUT) && (file != _LLIO_STDERR))
    {
        return -1;
    }

    for (i = 0; i < len; i++) {
        board_putc(ptr[i]);
    }

    return len;
}

long __lseek(int handle, long offset, int whence)
{
    return -1;
}

int __close(int handle)
{
    return 0;
}

int remove(const char * filename)
{
    return 0;
}