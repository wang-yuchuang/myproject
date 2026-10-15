
LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifneq ($(wildcard $(COREDIR)/libcpu.a),)
ABS_DIR :=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
GLOBAL_LDFLAGS += \
	-L$(ABS_DIR) -lcpu
endif

MODULE_SRCS += \
    $(LOCAL_DIR)/board.c \
    $(PRJDIR)/main.c

GLOBAL_INCLUDES += \
    $(LOCAL_DIR) \
    $(PRJDIR)

include $(PRJDIR)/project.mk

include $(MKROOT)/module.mk
