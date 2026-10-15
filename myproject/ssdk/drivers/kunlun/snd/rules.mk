LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_I2S_MC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/i2s/sdrv_i2s_mc.c
endif

ifeq ($(CONFIG_SDRV_SPDIF), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/i2s/sdrv_spdif.c
endif

ifeq ($(CONFIG_CDNS_I2S_SC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/i2s/cdns_i2s_sc.c
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
