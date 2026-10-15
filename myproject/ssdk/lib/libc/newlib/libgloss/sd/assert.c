#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <debug.h>

/* func can be NULL, in which case no function information is given.  */
void
_DEFUN (__assert_func, (file, line, func, failedexpr),
	const char *file _AND
	int line _AND
	const char *func _AND
	const char *failedexpr)
{
  ssdk_panic((const uint8_t *)file, line);
  while (1);
}

void
_DEFUN (__assert, (file, line, failedexpr),
	const char *file _AND
	int line _AND
	const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
  /* NOTREACHED */
}
