#include <cmsis_os2.h>
#include <sys/errno.h>
#include <sys/lock.h>

static int
_lock_init_(void **lock, uint32_t flags)
{
  osMutexAttr_t attr = {};

  if (!lock)
    return -EINVAL;

  if (*lock)
    return 0;

  attr.name = "newlib_lock";
  attr.attr_bits = flags;

  osMutexId_t newlock = osMutexNew(&attr);
  if (!newlock)
    return -ENOMEM;

  if (__sync_val_compare_and_swap (lock, NULL, newlock))
    osMutexDelete (newlock);

  return 0;
}

static int
_lock_close_(void **lock)
{
  if (!lock)
    return -EINVAL;

  if (*lock) {
    osMutexDelete(*lock);
    *lock = 0;
  }

  return 0;
}

static int
_lock_acquire_(void **lock, uint32_t flags, uint32_t timeout)
{
  int ret = _lock_init_(lock, flags);

  if (ret < 0)
    return ret;

  ret = osMutexAcquire(*lock, timeout);
  if (ret == osErrorTimeout)
      return -ETIMEDOUT;

  return ret;
}

static int
_lock_release_(void **lock)
{
  if (!lock)
    return -EINVAL;

  if (*lock)
    osMutexRelease(*lock);

  return 0;
}

int
_lock_init(_LOCK_T *lock)
{
  return _lock_init_(lock, 0);
}

int
_lock_init_recursive(_LOCK_RECURSIVE_T *lock)
{
  return _lock_init_(lock, osMutexRecursive);
}

int
_lock_close(_LOCK_T *lock)
{
  return _lock_close_(lock);
}

int
_lock_close_recursive(_LOCK_RECURSIVE_T *lock)
{
  return _lock_close_(lock);
}

int
_lock_acquire(_LOCK_T *lock)
{
  return _lock_acquire_(lock, 0, osWaitForever);
}

int
_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
  return _lock_acquire_(lock, osMutexRecursive, osWaitForever);
}

int
_lock_try_acquire(_LOCK_T *lock)
{
  return _lock_acquire_(lock, 0, 0);
}

int
_lock_try_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
  return _lock_acquire_(lock, osMutexRecursive, 0);
}

int
 _lock_release(_LOCK_T *lock)
{
  return _lock_release_(lock);
}

int
_lock_release_recursive(_LOCK_RECURSIVE_T *lock)
{
  return _lock_release_(lock);
}
