#include <cmsis_os2.h>
#include <errno.h>
#include <malloc.h>
#include <reent.h>
#include <string.h>

void *
_calloc_r (struct _reent *r, size_t n, size_t s)
{
  void *p = _malloc_r (r, s *= n);
  if (p)
    memset (p, 0, s);
  return p;
}

void
_free_r (struct _reent *r, void *p)
{
  osFree (p);
}

void *
_malloc_r (struct _reent *r, size_t s)
{
  void *p = osAlloc (s);
  if (!p)
    r->_errno = ENOMEM;

  return p;
}

void *
_memalign_r (struct _reent *r, size_t a, size_t s)
{
  void *p = osAllocAlign (a, s);
  if (!p)
    r->_errno = ENOMEM;

  return p;
}

void *
_realloc_r (struct _reent *r, void *p, size_t s)
{
  void *np;

  if (!p)
    return _malloc_r (r, s);

  if (!s) {
    _free_r (r, p);
    return 0;
  }

  np = _malloc_r (r, s);
  if (np) {
    memcpy (np, p, s);
    _free_r (r, p);
  }

  return np;
}
