# the above include may override SSDK_ROOT and SSDK_INC to allow external
# directories to be included in the build

SSDK_MAKEROOT ?= .
SSDK_ROOT ?= .
SSDK_INC ?= .
SSDK_LLD ?= drivers/lld
BUILDROOT ?= ./out
DEFAULT_PROJECT ?=
TOOLCHAIN_PREFIX ?=
MKROOT ?= mk

# specify SSDK_TOOLCHAIN according to MAKECMDGOALS, "gcc" by default
SSDK_TOOLCHAIN := $(findstring iar,$(MAKECMDGOALS))
ifeq ($(SSDK_TOOLCHAIN), )
SSDK_TOOLCHAIN := $(findstring rapidstudio,$(MAKECMDGOALS))
endif

ifneq ($(TOOLCHAIN), )
SSDK_TOOLCHAIN := $(TOOLCHAIN)
else
SSDK_TOOLCHAIN := gcc
endif

# check if SSDK_ROOT is already a part of SSDK_INC list and add it only if it is not
ifeq ($(filter $(SSDK_ROOT),$(SSDK_INC)), )
SSDK_INC := include $(SSDK_INC) $(SSDK_ROOT)
endif

export SSDK_MAKEROOT
export SSDK_ROOT
export SSDK_INC
export SSDK_LLD
export BUILDROOT
export DEFAULT_PROJECT
export TOOLCHAIN_PREFIX
export SSDK_TOOLCHAIN
export MKROOT

# veneer makefile that calls into the engine with ssdk as the build root
# if we're the top level invocation, call ourselves with additional args
_top:
	@$(MAKE) -C $(SSDK_MAKEROOT) -rR -f $(MKROOT)/engine.mk $(addprefix -I,$(SSDK_INC)) $(MAKECMDGOALS)

# If any arguments were provided, create a recipe for them that depends
# on the _top rule (thus calling it), but otherwise do nothing.
# "@:" (vs empty rule ";") prevents extra "'foo' is up to date." messages from
# being emitted.
$(MAKECMDGOALS): _top
	@:

.PHONY: _top
