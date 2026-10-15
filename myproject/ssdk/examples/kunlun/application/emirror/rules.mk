LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
    ${LOCAL_DIR}/src \
    $(LOCAL_DIR)/inc \

MODULE_SRCS += \
    $(LOCAL_DIR)/src/emirror.c \
    $(LOCAL_DIR)/src/emirror_camera.c \
    $(LOCAL_DIR)/src/emirror_csi.c \
    $(LOCAL_DIR)/src/emirror_player.c \

ifeq ($(CONFIG_USE_GUI_LVGL), y)
MODULE_SRCS += $(LOCAL_DIR)/src/emirror_lvgl.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable -Wno-maybe-uninitialized

include $(MKROOT)/module.mk
