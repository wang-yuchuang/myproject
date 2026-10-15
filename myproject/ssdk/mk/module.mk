
# modules
#
# args:
# MODULE : module name (required)
# MODULE_SRCS : list of source files, local path (required)
# MODULE_STATIC_LIB : if true generate .a instead of .o
# MODULE_DEPS : other modules that this one depends on
# MODULE_DEFINES : #defines local to this module
# MODULE_OPTFLAGS : OPTFLAGS local to this module
# MODULE_COMPILEFLAGS : COMPILEFLAGS local to this module
# MODULE_CFLAGS : CFLAGS local to this module
# MODULE_CPPFLAGS : CPPFLAGS local to this module
# MODULE_ASMFLAGS : ASMFLAGS local to this module
# MODULE_INCLUDES : include directories local to this module
# MODULE_SRCDEPS : extra dependencies that all of this module's files depend on
# MODULE_EXTRA_OBJS : extra .o files that should be linked with the module

# MODULE_ARM_OVERRIDE_SRCS : list of source files, local path that should be force compiled with ARM (if applicable)

# the minimum module rules.mk file is as follows:
#
# LOCAL_DIR := $(GET_LOCAL_DIR)
# MODULE := $(LOCAL_DIR)
#
# MODULE_SRCS := $(LOCAL_DIR)/at_least_one_source_file.c
#
# include make/module.mk


# for IAR support
# backup iar need flag
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_MODULE_INCLUDE += $(MODULE_INCLUDES)
IAR_MODULE_SRCS += $(MODULE_ARM_OVERRIDE_SRCS)
endif

ifeq ($(SSDK_TOOLCHAIN),rapidstudio)
IDE_MODULE_INCLUDE += $(MODULE_INCLUDES)
IDE_MODULE_SRCS += $(MODULE_ARM_OVERRIDE_SRCS)
endif

# test for old style rules.mk
ifneq ($(MODULE_OBJS),)
$(warning MODULE_OBJS = $(MODULE_OBJS))
$(error MODULE $(MODULE) is setting MODULE_OBJS, change to MODULE_SRCS)
endif
ifneq ($(OBJS),)
$(warning OBJS = $(OBJS))
$(error MODULE $(MODULE) is probably setting OBJS, change to MODULE_SRCS)
endif

MODULE_SRCDIR := $(MODULE)
MODULE_BUILDDIR := $(call TOBUILDDIR,$(MODULE_SRCDIR))

# add a local include dir to the global include path
GLOBAL_INCLUDES += $(MODULE_SRCDIR)/include

# add the listed module deps to the global list
MODULES += $(MODULE_DEPS)

#$(info module $(MODULE))
#$(info MODULE_SRCDIR $(MODULE_SRCDIR))
#$(info MODULE_BUILDDIR $(MODULE_BUILDDIR))
#$(info MODULE_DEPS $(MODULE_DEPS))
#$(info MODULE_SRCS $(MODULE_SRCS))

MODULE_INCLUDES := $(addprefix -I,$(MODULE_INCLUDES))

# include the rules to compile the module's object files
include $(MKROOT)/compile.mk

# MODULE_OBJS is passed back from compile.mk
#$(info MODULE_OBJS = $(MODULE_OBJS))

# build a ld -r style combined object
ifeq (true,$(call TOBOOL,$(MODULE_STATIC_LIB)))
ifeq ($(findstring arm64,$(MODULE_SRCDIR)),arm64)
MODULE_OBJECT := $(addprefix $(COREDIR)/,libcpu.a)
# generate git commit id
BUILD_LIBCOMMIT_ID := $(shell git rev-parse --short HEAD)
# generate time info
BUILD_LIBTIMESTAMP := $(shell date +"%F %T")
ifeq ($(CONFIG_ARCH_WITH_SMP), y)
BUILD_SMPINFO := SMP
else
BUILD_SMPINFO :=
endif
$(MODULE_OBJECT): $(MODULE_OBJS) $(MODULE_EXTRA_OBJS)
	@echo creating $@
	$(NOECHO)rm -f $@
	@echo 'const char libcpu_msg[] = "Libcpu version: g${BUILD_LIBCOMMIT_ID} - ${BUILD_LIBTIMESTAMP}${BUILD_SMPINFO}";' > $(BUILDDIR)/libcpu_msg.c
	$(CC) $(GLOBAL_OPTFLAGS) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(GLOBAL_CFLAGS) \
		$(ARCH_CFLAGS) $(THUMBCFLAGS) $(GLOBAL_INCLUDES) -c $(BUILDDIR)/libcpu_msg.c -o $(BUILDDIR)/build_msg.o
	$(NOECHO)$(AR) rcs $@ $(BUILDDIR)/build_msg.o $^
else ifeq ($(findstring FuncSafetyPkg,$(MODULE_SRCDIR)),FuncSafetyPkg)
MODULE_OBJECT := $(addprefix $(COREDIR)/,libFuncSafetyPkg.a)
$(MODULE_OBJECT): $(MODULE_OBJS) $(MODULE_EXTRA_OBJS)
	@$(MKDIR)
	@echo libFuncSafetyPkg creating $@
	$(NOECHO)rm -f $@
	$(NOECHO)$(AR) rcs $@ $^
else
MODULE_OBJECT := $(call TOBUILDDIR,$(MODULE_SRCDIR).mod.a)
$(MODULE_OBJECT): $(MODULE_OBJS) $(MODULE_EXTRA_OBJS)
	@$(MKDIR)
	@echo creating $@
	$(NOECHO)rm -f $@
	$(NOECHO)$(AR) rcs $@ $^
endif
else

ifneq ($(MODULE_SRCDIR), )
MODULE_OBJECT := $(call TOBUILDDIR,$(MODULE_SRCDIR).mod.o)
$(MODULE_OBJECT): $(MODULE_OBJS) $(MODULE_EXTRA_OBJS)
	@$(MKDIR)
	@echo linking $@
	$(NOECHO)$(LD) -r $^ -o $@ $(GLOBAL_MODULE_LDFLAGS)
endif
endif

# track all of the source files compiled
ALLSRCS += $(MODULE_SRCS)

# track all the objects built
ALLOBJS += $(MODULE_OBJS)

# track the module object for make clean
GENERATED += $(MODULE_OBJECT)

# make the rest of the build depend on our output
ALLMODULE_OBJS := $(ALLMODULE_OBJS) $(MODULE_OBJECT)

# empty out any vars set here
MODULE :=
MODULE_SRCDIR :=
MODULE_BUILDDIR :=
MODULE_DEPS :=
MODULE_SRCS :=
MODULE_STATIC_LIB :=
MODULE_OBJS :=
MODULE_DEFINES :=
MODULE_OPTFLAGS :=
MODULE_COMPILEFLAGS :=
MODULE_CFLAGS :=
MODULE_CPPFLAGS :=
MODULE_ASMFLAGS :=
MODULE_SRCDEPS :=
MODULE_INCLUDES :=
MODULE_EXTRA_OBJS :=
MODULE_CONFIG :=
MODULE_OBJECT :=
MODULE_ARM_OVERRIDE_SRCS :=
