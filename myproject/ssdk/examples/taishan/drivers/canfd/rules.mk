LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_CANFD_BOARD_TEST), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/can_test.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/can_demo.c
endif

include $(MKROOT)/module.mk