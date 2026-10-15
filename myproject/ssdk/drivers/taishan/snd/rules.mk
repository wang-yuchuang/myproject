LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SACI_I2S), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/i2s/sdrv_saci_i2s.c
endif

ifeq ($(CONFIG_SACI_PDM), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/pdm/sdrv_saci_pdm.c
endif

ifeq ($(CONFIG_CODEC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/codec_driver.c
endif

ifeq ($(CONFIG_CODEC_AK4619), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/codecs/ak4619.c
endif

ifeq ($(CONFIG_CODEC_TLV320), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/codecs/tlv320.c
endif

include $(MKROOT)/module.mk
