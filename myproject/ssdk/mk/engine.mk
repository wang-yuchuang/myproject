-include local.mk
include $(MKROOT)/macros.mk

ifndef BOARD
$(error couldn't find board or board doesn't define target)
endif

$(info BOARD = $(BOARD))
$(info CORE = $(CORE))
$(info PROJECT = $(PROJECT))
$(info SSDK_TOOLCHAIN = $(SSDK_TOOLCHAIN))
export BOARD
export CORE
export PROJECT

$(info OSTYPE = $(OSTYPE))
ifeq ($(OSTYPE),msys)
OS := windows
else
OS := linux
endif

BOARDDIR := boards/$(BOARD)
ifneq ($(CORE),)
COREDIR := $(BOARDDIR)/$(CORE)
BUILDDIR=${BOARDDIR}/build/${PROJECT}/${SSDK_TOOLCHAIN}/${CORE}
OUTBIN := $(BUILDDIR)/$(CORE).bin
OUTELF := $(BUILDDIR)/$(CORE).elf
else
COREDIR := $(BOARDDIR)
BUILDDIR=${BOARDDIR}/build/${PROJECT}/${SSDK_TOOLCHAIN}
OUTBIN := $(BUILDDIR)/$(PROJECT).bin
OUTELF := $(BUILDDIR)/$(PROJECT).elf
endif
PRJDIR := $(if $(PROJECT_SRC_DIR),$(PROJECT_SRC_DIR),$(COREDIR)/projects/$(PROJECT))

include $(PRJDIR)/.config

ARCH := $(patsubst "%",%,$(strip $(CONFIG_ARCH)))

KCONFIG_HEADER := $(PRJDIR)/config.h
KCONFIG_MK := $(PRJDIR)/.config

GLOBAL_KERNEL_INCLUDES := $(addsuffix /include,$(SSDK_INC))
# For backwards compatibility.
#GLOBAL_KERNEL_INCLUDES += $(addsuffix /include/uapi/uapi,$(SSDK_INC)) $(addsuffix /include/shared/ssdk,$(SSDK_INC))
#GLOBAL_UAPI_INCLUDES := $(addsuffix /include/uapi,$(SSDK_INC))
#GLOBAL_SHARED_INCLUDES := $(addsuffix /include/shared,$(SSDK_INC))
#GLOBAL_USER_INCLUDES := $(addsuffix /include/user,$(SSDK_INC))
#GLOBAL_INCLUDES := $(BUILDDIR) $(GLOBAL_UAPI_INCLUDES) $(GLOBAL_SHARED_INCLUDES) $(GLOBAL_KERNEL_INCLUDES)
GLOBAL_INCLUDES := $(BUILDDIR) $(GLOBAL_KERNEL_INCLUDES)
GLOBAL_OPTFLAGS ?= $(ARCH_OPTFLAGS)
GLOBAL_COMPILEFLAGS := -g -finline -include $(KCONFIG_HEADER)
GLOBAL_COMPILEFLAGS += -fno-common
GLOBAL_CFLAGS := --std=c11 -Wwrite-strings -Wno-discarded-qualifiers
GLOBAL_CPPFLAGS := --std=c++11 -fno-exceptions -fno-rtti -fno-threadsafe-statics
#GLOBAL_CPPFLAGS += -Weffc++
GLOBAL_ASMFLAGS := -DASSEMBLY
GLOBAL_LDFLAGS :=

GLOBAL_LDFLAGS += $(addprefix -L,$(SSDK_INC))

# Architecture specific compile flags
ARCH_COMPILEFLAGS :=
ARCH_CFLAGS :=
ARCH_CPPFLAGS :=
ARCH_ASMFLAGS :=
ARCH_TOOLCHAIN := $(SSDK_TOOLCHAIN)

# top level rule
all:: $(OUTBIN) $(OUTELF).lst $(OUTELF).debug.lst $(OUTELF).sym $(OUTELF).sym.sorted $(OUTELF).size $(OUTELF).dump $(BUILDDIR)/srcfiles.txt $(BUILDDIR)/include_paths.txt

# master module object list
ALLOBJS_MODULE :=

# master object list (for dep generation)
ALLOBJS :=

# master source file list
ALLSRCS :=

# a linker script needs to be declared in one of the project/target/platform files
LINKER_SCRIPT :=

# anything you add here will be deleted in make clean
GENERATED :=

# Anything added to GLOBAL_SRCDEPS will become a dependency of every source file in the system.
# Useful for header files that may be included by one or more source files.
GLOBAL_SRCDEPS := $(KCONFIG_HEADER)

# these need to be filled out by the project/target/platform rules.mk files
ALLMODULES :=

# add any external module dependencies
MODULES := $(EXTERNAL_MODULES)

# any .mk specified here will be included before build.mk
EXTRA_BUILDRULES :=

# any rules you put here will also be built by the system before considered being complete
EXTRA_BUILDDEPS :=

# any rules you put here will be depended on in clean builds
EXTRA_CLEANDEPS :=

# any objects you put here get linked with the final image
EXTRA_OBJS :=

# any extra linker scripts to be put on the command line
EXTRA_LINKER_SCRIPTS :=

# if someone defines this, the build id will be pulled into lib/version
BUILDID ?=

# comment out or override if you want to see the full output of each command
NOECHO ?= @

# default to building with clang
#CLANGBUILD ?= true
#override CLANGBUILD := $(call TOBOOL,$(CLANGBUILD))

#ifeq ($(call TOBOOL,$(CLANGBUILD)), true)
#GLOBAL_COMPILEFLAGS += -Wimplicit-fallthrough
#endif

# common scripts for the project
# ===================================================================
# gcc specialized scripts
ifeq ($(SSDK_TOOLCHAIN),gcc)
ifeq ($(CONFIG_LIBC),y)
GLOBAL_CFLAGS += -DCONFIG_NEWLIB
endif

ifneq ($(filter $(CONFIG_ARMV7R_TCMA_BASE), 0 0x0), )
GLOBAL_COMPILEFLAGS += \
    -DOS_CRITICAL_CODE=__attribute__\(\(section\(\"\.tcmcode\"\)\)\) \
    -DOS_CRITICAL_DATA=__attribute__\(\(section\(\"\.tcmdata\"\)\)\)
else
GLOBAL_COMPILEFLAGS += \
    -DOS_CRITICAL_CODE= \
    -DOS_CRITICAL_DATA=
endif
endif

# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_GLB_DEF :=
IAR_MODULE_INCLUDE :=
IAR_MODULE_SRCS :=
ifeq ($(CONFIG_LIBC),y)
IAR_GLB_DEF += CONFIG_IARLIB
endif

ifneq ($(filter $(CONFIG_ARMV7R_TCMA_BASE), 0 0x0), )
IAR_GLB_DEF += \
    OS_CRITICAL_CODE=__attribute__((section(\".tcmcode\"))) \
    OS_CRITICAL_DATA=__attribute__((section(\".tcmdata\")))
else
IAR_GLB_DEF += \
    OS_CRITICAL_CODE= \
    OS_CRITICAL_DATA=
endif
endif

# for rapidstudio support
ifeq ($(SSDK_TOOLCHAIN),rapidstudio)
IDE_MODULE_INCLUDE :=
IDE_MODULE_SRCS :=
export IDE_MODULE_INCLUDE
export IDE_MODULE_SRCS
ifeq ($(CONFIG_LIBC),y)
GLOBAL_CFLAGS += -DCONFIG_NEWLIB
endif
endif

# ===================================================================

# default to building with clang for google prebuild gcc
#
ifneq ($(findstring android, $(ARCH_$(ARCH)_TOOLCHAIN_PREFIX)), )
CLANGBUILD ?= true
override CLANGBUILD := $(call TOBOOL,$(CLANGBUILD))
endif

ifeq ($(call TOBOOL,$(CLANGBUILD)), true)
GLOBAL_COMPILEFLAGS += -Wimplicit-fallthrough
endif

ifeq ($(findstring android, $(ARCH_$(ARCH)_TOOLCHAIN_PREFIX)), )
GLOBAL_COMPILEFLAGS += -Wno-nonnull-compare
endif

ifeq ($(ARCH),arm)
GLOBAL_COMPILEFLAGS += -Werror -Wall -Wsign-compare -fno-short-enums
GLOBAL_COMPILEFLAGS += -Wno-parentheses
else
GLOBAL_COMPILEFLAGS += -Wall -Wsign-compare -fno-short-enums
endif
GLOBAL_COMPILEFLAGS += -Wno-multichar -Wno-unused-function -Wno-unused-label -Wno-tautological-compare -Wno-discarded-qualifiers -Wno-sign-compare

$(info $(ARCH))

ifeq ($(VERIFIED_BOOT), true)
CONFIG_VERIFIED_BOOT=y
CONFIG_SILEX_CRYPTO=y
CONFIG_SD_X509=y
CONFIG_LIBAVB=y
CONFIG_CRYPTO=y
CONFIG_SD_CRC32=y
CONFIG_DISK=y
CONFIG_FUSE=y
CONFIG_KUNLUN_FUSE=y
CONFIG_PARTITION=y
CONFIG_PART_LOAD_UTIL=y
CONFIG_PARTITION_BLOCK_IO_MODE=y
GLOBAL_CFLAGS += -DCONFIG_VERIFIED_BOOT=1
GLOBAL_CFLAGS += -DCONFIG_SILEX_CRYPTO=1
GLOBAL_CFLAGS += -DCONFIG_SD_X509=1
GLOBAL_CFLAGS += -DCONFIG_LIBAVB=1
GLOBAL_CFLAGS += -DCONFIG_CRYPTO=1
GLOBAL_CFLAGS += -DCONFIG_SD_CRC32=1
GLOBAL_CFLAGS += -DCONFIG_DISK=1
GLOBAL_CFLAGS += -DCONFIG_FUSE=1
GLOBAL_CFLAGS += -DCONFIG_KUNLUN_FUSE=1
GLOBAL_CFLAGS += -DCONFIG_PARTITION=1
GLOBAL_CFLAGS += -DCONFIG_PART_LOAD_UTIL=1
GLOBAL_CFLAGS += -DCONFIG_PARTITION_BLOCK_IO_MODE=1

endif

# try to include the project file
include $(KCONFIG_MK)
include arch/$(ARCH)/rules.mk
include lib/rules.mk
include drivers/rules.mk
include rtos/rules.mk
include boards/rules.mk
include middleware/rules.mk
include examples/rules.mk
ifeq ($(CONFIG_SSDK_TEST), y)
include test/rules.mk
endif
ifeq ($(CONFIG_VALIDATION), y)
include validation/rules.mk
endif

# generate config build dir
include $(MKROOT)/configs.mk

# recursively include any modules in the MODULE variable, leaving a trail of included
# modules in the ALLMODULES list
include $(MKROOT)/recurse.mk

# any extra top level build dependencies that someone declared
all:: $(EXTRA_BUILDDEPS)

# prefix all of the paths in GLOBAL_INCLUDES with -I
GLOBAL_INCLUDES := $(addprefix -I,$(GLOBAL_INCLUDES))

# test for some old variables
ifneq ($(INCLUDES),)
$(error INCLUDES variable set, please move to GLOBAL_INCLUDES: $(INCLUDES))
endif

# default to no ccache
CCACHE ?=
ifeq ($(call TOBOOL,$(CLANGBUILD)), true)
ifeq ($(CLANG_BINDIR),)
$(error clang directory not specified, please set CLANG_BINDIR)
endif
CC := $(CCACHE) $(CLANG_BINDIR)/clang
GLOBAL_COMPILEFLAGS += -no-integrated-as
else
CC := $(CCACHE) $(TOOLCHAIN_PREFIX)gcc
endif
LD := $(TOOLCHAIN_PREFIX)ld.bfd
AR := $(TOOLCHAIN_PREFIX)ar
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
CPPFILT := $(TOOLCHAIN_PREFIX)c++filt
SIZE := $(TOOLCHAIN_PREFIX)size
NM := $(TOOLCHAIN_PREFIX)nm
STRIP := $(TOOLCHAIN_PREFIX)strip

GLOBAL_COMPILEFLAGS += -U__INT32_TYPE__ -U__UINT32_TYPE__ -D__GNU_LIBRARY__ -D__USE_GNU

LIBGCC += $(shell $(CC) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(THUMBCFLAGS) -print-file-name=libm.a)
LIBGCC += $(shell $(CC) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(THUMBCFLAGS) -print-file-name=libgcc.a)

# try to have the compiler output colorized error messages if available
export GCC_COLORS ?= 1

# the logic to compile and link stuff is in here
include $(MKROOT)/build.mk

DEPS := $(ALLOBJS:%o=%d)

ifneq ($(OBJS),)
$(warning OBJS=$(OBJS))
$(error OBJS is not empty, please convert to new module format)
endif
ifneq ($(OPTFLAGS),)
$(warning OPTFLAGS=$(OPTFLAGS))
$(error OPTFLAGS is not empty, please use GLOBAL_OPTFLAGS or MODULE_OPTFLAGS)
endif
ifneq ($(CFLAGS),)
$(warning CFLAGS=$(CFLAGS))
$(error CFLAGS is not empty, please use GLOBAL_CFLAGS or MODULE_CFLAGS)
endif
ifneq ($(CPPFLAGS),)
$(warning CPPFLAGS=$(CPPFLAGS))
$(error CPPFLAGS is not empty, please use GLOBAL_CPPFLAGS or MODULE_CPPFLAGS)
endif

$(info LIBGCC = $(LIBGCC))
$(info GLOBAL_COMPILEFLAGS = $(GLOBAL_COMPILEFLAGS))
$(info GLOBAL_OPTFLAGS = $(GLOBAL_OPTFLAGS))

# make all object files depend on any targets in GLOBAL_SRCDEPS
$(ALLOBJS): $(GLOBAL_SRCDEPS) $(CONFIG_HEADER_FILES)

clean: $(EXTRA_CLEANDEPS)
	rm -f $(ALLOBJS) $(DEPS) $(GENERATED) $(OUTBIN) $(OUTELF) $(OUTELF).lst $(OUTELF).debug.lst $(OUTELF).sym $(OUTELF).sym.sorted $(OUTELF).size $(OUTELF).hex $(OUTELF).dump

# for IAR support
ifeq ($(SSDK_TOOLCHAIN),iar)
include mk/iar/Makefile.iar
endif

# for rapidstudio support
ifeq ($(SSDK_TOOLCHAIN),rapidstudio)
include mk/rapidstudio/Makefile.ide
endif

# Empty rule for the .d files. The above rules will build .d files as a side
# effect. Only works on gcc 3.x and above, however.
%.d:

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(DEPS)
endif
