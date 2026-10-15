#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

typedef void *_LOCK_T;
typedef void *_LOCK_RECURSIVE_T;

#define __LOCK_INIT(class, lock)            class _LOCK_T lock;
#define __LOCK_INIT_RECURSIVE(class, lock)  class _LOCK_RECURSIVE_T lock;
#define __lock_init(lock)                   _lock_init (&(lock))
#define __lock_init_recursive(lock)         _lock_init_recursive (&(lock))
#define __lock_close(lock)                  _lock_close (&(lock))
#define __lock_close_recursive(lock)        _lock_close_recursive (&(lock))
#define __lock_acquire(lock)                _lock_acquire (&(lock))
#define __lock_acquire_recursive(lock)      _lock_acquire_recursive (&(lock))
#define __lock_try_acquire(lock)            _lock_try_acquire (&(lock))
#define __lock_try_acquire_recursive(lock)  _lock_try_acquire_recursive (&(lock))
#define __lock_release(lock)                _lock_release (&(lock))
#define __lock_release_recursive(lock)      _lock_release_recursive (&(lock))

extern int _lock_init (_LOCK_T *lock);
extern int _lock_init_recursive (_LOCK_RECURSIVE_T *lock);
extern int _lock_close (_LOCK_T *lock);
extern int _lock_close_recursive (_LOCK_RECURSIVE_T *lock);
extern int _lock_acquire (_LOCK_T *lock);
extern int _lock_acquire_recursive (_LOCK_RECURSIVE_T *lock);
extern int _lock_try_acquire (_LOCK_T *lock);
extern int _lock_try_acquire_recursive (_LOCK_RECURSIVE_T *lock);
extern int _lock_release (_LOCK_T *lock);
extern int _lock_release_recursive (_LOCK_RECURSIVE_T *lock);

#endif /* __SYS_LOCK_H__ */
