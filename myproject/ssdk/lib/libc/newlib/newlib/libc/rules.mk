LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_COMPILEFLAGS += -Wno-unused-but-set-variable

ifeq ($(ARCH),arm)
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/machine/arm

MODULE_SRCS += \
	$(LOCAL_DIR)/machine/arm/memchr.S \
	$(LOCAL_DIR)/machine/arm/memchr-stub.c \
	$(LOCAL_DIR)/machine/arm/memcpy.S \
	$(LOCAL_DIR)/machine/arm/memcpy-stub.c \
	$(LOCAL_DIR)/machine/arm/memset.S \
	$(LOCAL_DIR)/machine/arm/memset-stub.c \
	$(LOCAL_DIR)/machine/arm/setjmp.S \
	$(LOCAL_DIR)/machine/arm/strcmp.S \
	$(LOCAL_DIR)/machine/arm/strcpy.c \
	$(LOCAL_DIR)/machine/arm/strlen.S \
	$(LOCAL_DIR)/machine/arm/strlen-stub.c
endif

ifeq ($(ARCH),arm64)

#when mmu doesn't enable, must set this flag.
#MODULE_COMPILEFLAGS += -mstrict-align -DPREFER_SIZE_OVER_SPEED

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/machine/aarch64

MODULE_SRCS += \
	$(LOCAL_DIR)/machine/aarch64/memchr.S \
	$(LOCAL_DIR)/machine/aarch64/memchr-stub.c \
	$(LOCAL_DIR)/machine/aarch64/memcmp.S \
	$(LOCAL_DIR)/machine/aarch64/memcmp-stub.c \
	$(LOCAL_DIR)/machine/aarch64/memcpy.S \
	$(LOCAL_DIR)/machine/aarch64/memcpy-stub.c \
	$(LOCAL_DIR)/machine/aarch64/memmove.S \
	$(LOCAL_DIR)/machine/aarch64/memmove-stub.c \
	$(LOCAL_DIR)/machine/aarch64/memset.S \
	$(LOCAL_DIR)/machine/aarch64/memset-stub.c \
	$(LOCAL_DIR)/machine/aarch64/rawmemchr.S \
	$(LOCAL_DIR)/machine/aarch64/rawmemchr-stub.c \
	$(LOCAL_DIR)/machine/aarch64/setjmp.S \
	$(LOCAL_DIR)/machine/aarch64/stpcpy.S \
	$(LOCAL_DIR)/machine/aarch64/stpcpy-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strchrnul.S \
	$(LOCAL_DIR)/machine/aarch64/strchrnul-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strchr.S \
	$(LOCAL_DIR)/machine/aarch64/strchr-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strcmp.S \
	$(LOCAL_DIR)/machine/aarch64/strcmp-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strcpy.S \
	$(LOCAL_DIR)/machine/aarch64/strcpy-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strlen.S \
	$(LOCAL_DIR)/machine/aarch64/strlen-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strncmp.S \
	$(LOCAL_DIR)/machine/aarch64/strncmp-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strnlen.S \
	$(LOCAL_DIR)/machine/aarch64/strnlen-stub.c \
	$(LOCAL_DIR)/machine/aarch64/strrchr.S \
	$(LOCAL_DIR)/machine/aarch64/strrchr-stub.c
endif
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \
	$(LOCAL_DIR)/stdlib

ifeq ($(ARCH),arm)
MODULE_SRCS += \
	$(LOCAL_DIR)/ctype/ctype_.c \
	$(LOCAL_DIR)/errno/errno.c \
	$(LOCAL_DIR)/locale/locale.c \
	$(LOCAL_DIR)/locale/localeconv.c \
	$(LOCAL_DIR)/reent/impure.c \
	$(LOCAL_DIR)/reent/reent.c \
	$(LOCAL_DIR)/stdio/clearerr.c \
	$(LOCAL_DIR)/stdio/fclose.c \
	$(LOCAL_DIR)/stdio/feof.c \
	$(LOCAL_DIR)/stdio/ferror.c \
	$(LOCAL_DIR)/stdio/fflush.c \
	$(LOCAL_DIR)/stdio/fgetc.c \
	$(LOCAL_DIR)/stdio/fgetpos.c \
	$(LOCAL_DIR)/stdio/fgets.c \
	$(LOCAL_DIR)/stdio/findfp.c \
	$(LOCAL_DIR)/stdio/fileno.c \
	$(LOCAL_DIR)/stdio/flags.c \
	$(LOCAL_DIR)/stdio/fopen.c \
	$(LOCAL_DIR)/stdio/fprintf.c \
	$(LOCAL_DIR)/stdio/fputc.c \
	$(LOCAL_DIR)/stdio/fputs.c \
	$(LOCAL_DIR)/stdio/fread.c \
	$(LOCAL_DIR)/stdio/fseek.c \
	$(LOCAL_DIR)/stdio/fseeko.c \
	$(LOCAL_DIR)/stdio/fsetpos.c \
	$(LOCAL_DIR)/stdio/ftell.c \
	$(LOCAL_DIR)/stdio/ftello.c \
	$(LOCAL_DIR)/stdio/fvwrite.c \
	$(LOCAL_DIR)/stdio/fwalk.c \
	$(LOCAL_DIR)/stdio/fwrite.c \
	$(LOCAL_DIR)/stdio/makebuf.c \
	$(LOCAL_DIR)/stdio/putc.c \
	$(LOCAL_DIR)/stdio/refill.c \
	$(LOCAL_DIR)/stdio/rewind.c \
	$(LOCAL_DIR)/stdio/rget.c \
	$(LOCAL_DIR)/stdio/snprintf.c \
	$(LOCAL_DIR)/stdio/stdio.c \
	$(LOCAL_DIR)/stdio/sprintf.c \
	$(LOCAL_DIR)/stdio/vprintf.c \
	$(LOCAL_DIR)/stdio/vsnprintf.c \
	$(LOCAL_DIR)/stdio/vsprintf.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf-stringonly.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf_float.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf_i.c \
	$(LOCAL_DIR)/stdio/wbuf.c \
	$(LOCAL_DIR)/stdio/wsetup.c \
	$(LOCAL_DIR)/stdlib/atoi.c \
	$(LOCAL_DIR)/stdlib/atof.c \
	$(LOCAL_DIR)/stdlib/cxa_atexit.c \
	$(LOCAL_DIR)/stdlib/dtoa.c \
	$(LOCAL_DIR)/stdlib/dtoastub.c \
	$(LOCAL_DIR)/stdlib/environ.c \
	$(LOCAL_DIR)/stdlib/envlock.c \
	$(LOCAL_DIR)/stdlib/gdtoa-gethex.c \
	$(LOCAL_DIR)/stdlib/gdtoa-hexnan.c \
	$(LOCAL_DIR)/stdlib/itoa.c \
	$(LOCAL_DIR)/stdlib/mbtowc_r.c \
	$(LOCAL_DIR)/stdlib/mprec.c \
	$(LOCAL_DIR)/stdlib/rand.c \
	$(LOCAL_DIR)/stdlib/strtod.c \
	$(LOCAL_DIR)/stdlib/strtol.c \
	$(LOCAL_DIR)/stdlib/strtoul.c \
	$(LOCAL_DIR)/stdlib/strtoll.c \
	$(LOCAL_DIR)/stdlib/strtoull.c \
	$(LOCAL_DIR)/stdlib/utoa.c \
	$(LOCAL_DIR)/stdlib/wctomb_r.c \
	$(LOCAL_DIR)/stdlib/getopt.c \
	$(LOCAL_DIR)/string/memcmp.c \
	$(LOCAL_DIR)/string/memmove.c \
	$(LOCAL_DIR)/string/strcat.c \
	$(LOCAL_DIR)/string/strchr.c \
	$(LOCAL_DIR)/string/strdup.c \
	$(LOCAL_DIR)/string/strdup_r.c \
	$(LOCAL_DIR)/string/strerror.c \
	$(LOCAL_DIR)/string/strncat.c \
	$(LOCAL_DIR)/string/strncmp.c \
	$(LOCAL_DIR)/string/strncpy.c \
	$(LOCAL_DIR)/string/strndup.c \
	$(LOCAL_DIR)/string/strndup_r.c \
	$(LOCAL_DIR)/string/strrchr.c \
	$(LOCAL_DIR)/string/strstr.c \
	$(LOCAL_DIR)/string/strtok.c \
	$(LOCAL_DIR)/string/strtok_r.c \
	$(LOCAL_DIR)/string/u_strerr.c \
	$(LOCAL_DIR)/string/strnstr.c \
	$(LOCAL_DIR)/string/strnlen.c \
	$(LOCAL_DIR)/string/memmem.c \
	$(LOCAL_DIR)/string/strcasecmp.c \
	$(LOCAL_DIR)/stdlib/setenv.c \
	$(LOCAL_DIR)/stdlib/setenv_r.c \
	$(LOCAL_DIR)/stdlib/getenv.c \
	$(LOCAL_DIR)/stdlib/getenv_r.c \
	$(LOCAL_DIR)/stdlib/malign.c \
	$(LOCAL_DIR)/stdlib/malloc.c \
	$(LOCAL_DIR)/stdlib/calloc.c \
	$(LOCAL_DIR)/stdlib/realloc.c \
	$(LOCAL_DIR)/stdlib/reallocf.c
endif
ifeq ($(ARCH),arm64)
MODULE_SRCS += \
	$(LOCAL_DIR)/ctype/ctype_.c \
	$(LOCAL_DIR)/errno/errno.c \
	$(LOCAL_DIR)/locale/locale.c \
	$(LOCAL_DIR)/locale/localeconv.c \
	$(LOCAL_DIR)/reent/impure.c \
	$(LOCAL_DIR)/reent/reent.c \
	$(LOCAL_DIR)/stdio/clearerr.c \
	$(LOCAL_DIR)/stdio/fclose.c \
	$(LOCAL_DIR)/stdio/feof.c \
	$(LOCAL_DIR)/stdio/ferror.c \
	$(LOCAL_DIR)/stdio/fflush.c \
	$(LOCAL_DIR)/stdio/fgetc.c \
	$(LOCAL_DIR)/stdio/fgetpos.c \
	$(LOCAL_DIR)/stdio/fgets.c \
	$(LOCAL_DIR)/stdio/findfp.c \
	$(LOCAL_DIR)/stdio/fileno.c \
	$(LOCAL_DIR)/stdio/flags.c \
	$(LOCAL_DIR)/stdio/fopen.c \
	$(LOCAL_DIR)/stdio/fprintf.c \
	$(LOCAL_DIR)/stdio/fputc.c \
	$(LOCAL_DIR)/stdio/fputs.c \
	$(LOCAL_DIR)/stdio/fread.c \
	$(LOCAL_DIR)/stdio/fseek.c \
	$(LOCAL_DIR)/stdio/fseeko.c \
	$(LOCAL_DIR)/stdio/fsetpos.c \
	$(LOCAL_DIR)/stdio/ftell.c \
	$(LOCAL_DIR)/stdio/ftello.c \
	$(LOCAL_DIR)/stdio/fvwrite.c \
	$(LOCAL_DIR)/stdio/fwalk.c \
	$(LOCAL_DIR)/stdio/fwrite.c \
	$(LOCAL_DIR)/stdio/makebuf.c \
	$(LOCAL_DIR)/stdio/putc.c \
	$(LOCAL_DIR)/stdio/refill.c \
	$(LOCAL_DIR)/stdio/rewind.c \
	$(LOCAL_DIR)/stdio/rget.c \
	$(LOCAL_DIR)/stdio/snprintf.c \
	$(LOCAL_DIR)/stdio/stdio.c \
	$(LOCAL_DIR)/stdio/sprintf.c \
	$(LOCAL_DIR)/stdio/vprintf.c \
	$(LOCAL_DIR)/stdio/vsnprintf.c \
	$(LOCAL_DIR)/stdio/vsprintf.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf-stringonly.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf_float.c \
	$(LOCAL_DIR)/stdio/nano-vfprintf_i.c \
	$(LOCAL_DIR)/stdio/wbuf.c \
	$(LOCAL_DIR)/stdio/wsetup.c \
	$(LOCAL_DIR)/stdlib/atoi.c \
	$(LOCAL_DIR)/stdlib/dtoa.c \
	$(LOCAL_DIR)/stdlib/dtoastub.c \
	$(LOCAL_DIR)/stdlib/environ.c \
	$(LOCAL_DIR)/stdlib/envlock.c \
	$(LOCAL_DIR)/stdlib/itoa.c \
	$(LOCAL_DIR)/stdlib/mbtowc_r.c \
	$(LOCAL_DIR)/stdlib/mprec.c \
	$(LOCAL_DIR)/stdlib/rand.c \
	$(LOCAL_DIR)/stdlib/strtol.c \
	$(LOCAL_DIR)/stdlib/strtoul.c \
	$(LOCAL_DIR)/stdlib/strtoll.c \
	$(LOCAL_DIR)/stdlib/strtoull.c\
	$(LOCAL_DIR)/stdlib/utoa.c \
	$(LOCAL_DIR)/stdlib/wctomb_r.c \
	$(LOCAL_DIR)/stdlib/getopt.c \
	$(LOCAL_DIR)/string/strcat.c \
	$(LOCAL_DIR)/string/strdup.c \
	$(LOCAL_DIR)/string/strdup_r.c \
	$(LOCAL_DIR)/string/strerror.c \
	$(LOCAL_DIR)/string/strncat.c \
	$(LOCAL_DIR)/string/strncpy.c \
	$(LOCAL_DIR)/string/strndup.c \
	$(LOCAL_DIR)/string/strndup_r.c \
	$(LOCAL_DIR)/string/strstr.c \
	$(LOCAL_DIR)/string/strtok.c \
	$(LOCAL_DIR)/string/strtok_r.c \
	$(LOCAL_DIR)/string/u_strerr.c \
	$(LOCAL_DIR)/stdlib/setenv.c \
	$(LOCAL_DIR)/stdlib/setenv_r.c \
	$(LOCAL_DIR)/stdlib/getenv.c \
	$(LOCAL_DIR)/stdlib/getenv_r.c \
	$(LOCAL_DIR)/stdlib/malign.c \
	$(LOCAL_DIR)/stdlib/malloc.c \
	$(LOCAL_DIR)/stdlib/calloc.c \
	$(LOCAL_DIR)/stdlib/realloc.c \
	$(LOCAL_DIR)/stdlib/reallocf.c
endif

ifeq ($(CONFIG_STD_HEAP), y)
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/sys/rtems/include

MODULE_SRCS += \
	$(LOCAL_DIR)/stdlib/nano-mallocr.c \
	$(LOCAL_DIR)/stdlib/mallinfo.c
endif

include $(MKROOT)/module.mk
