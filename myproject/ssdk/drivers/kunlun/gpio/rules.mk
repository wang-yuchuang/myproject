LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_GPIO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_gpio.c
endif

ifeq ($(CONFIG_TCA9539), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/tca9539.c
endif

ifeq ($(CONFIG_TCA6408), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/tca6408.c
endif

include $(MKROOT)/module.mk
