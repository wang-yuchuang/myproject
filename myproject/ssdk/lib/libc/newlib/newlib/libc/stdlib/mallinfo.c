#ifdef MALLOC_PROVIDED

int _dummy_malloc = 1;

#else

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _REENT_ONLY

struct mallinfo
mallinfo(void)
{
    return _mallinfo_r(_REENT);
}

#endif

#endif /* ! defined (MALLOC_PROVIDED) */
