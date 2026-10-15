LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TOUCH_KUNLUN_PLATFORM), y)
MODULE_SRCS += \
	${LOCAL_DIR}/touch.c
endif

ifeq ($(CONFIG_TOUCH_KUNLUN_GOODIX), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/touch_goodix.c
endif

ifeq ($(CONFIG_TOUCH_KUNLUN_CYTTSP6), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/touch_cyttsp6.c
endif

include $(MKROOT)/module.mk
