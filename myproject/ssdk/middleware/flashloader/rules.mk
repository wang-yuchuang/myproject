LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/flash_loader_asm.s \
	$(LOCAL_DIR)/flash_loader.c \
	$(LOCAL_DIR)/flash_loader_implement.c \
	$(LOCAL_DIR)/flash_wrapper.c

ifeq ($(CONFIG_AVB_FOOTER), y)
MODULE_SRCS += $(LOCAL_DIR)/avb_footer.c
endif

ifeq ($(CONFIG_BPT_UPDATE), y)
MODULE_SRCS += $(LOCAL_DIR)/bpt_update.c
endif

include $(MKROOT)/module.mk
