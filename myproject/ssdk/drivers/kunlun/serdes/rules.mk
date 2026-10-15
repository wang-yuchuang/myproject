LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/serdes.c \

ifeq ($(CONFIG_SERDES_TI9XX), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/serdes_ti_9xx.c
endif

ifeq ($(CONFIG_SERDES_MAX967XX), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/serdes_max967xx.c
endif


include $(MKROOT)/module.mk
