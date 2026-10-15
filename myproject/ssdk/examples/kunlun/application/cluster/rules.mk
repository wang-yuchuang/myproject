LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
    $(LOCAL_DIR) \
    $(LOCAL_DIR)/inc

MODULE_SRCS += \
    $(LOCAL_DIR)/src/cluster.c

ifeq ($(CONFIG_USE_GUI_LVGL), y)
MODULE_SRCS += $(LOCAL_DIR)/src/cluster_lvgl.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
