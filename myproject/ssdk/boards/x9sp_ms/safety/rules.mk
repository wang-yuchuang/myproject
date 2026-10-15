
LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/board.c \
    $(PRJDIR)/main.c

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)

include $(PRJDIR)/project.mk

include $(MKROOT)/module.mk