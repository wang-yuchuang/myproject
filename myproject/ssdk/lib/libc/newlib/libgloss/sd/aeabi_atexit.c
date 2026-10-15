#include <stdlib.h>

void *__dso_handle;

extern int __cxa_atexit(void (*func)(void *), void *arg, void *d);

/* Register a function to be called by exit or when a shared library
   is unloaded.  This routine is like __cxa_atexit, but uses the
   calling sequence required by the ARM EABI.  */
int
__aeabi_atexit (void *arg, void (*func) (void *), void *d)
{
  return __cxa_atexit (func, arg, d);
}
