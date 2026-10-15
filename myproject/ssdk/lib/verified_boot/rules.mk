LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_VERIFIED_BOOT), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/ce_verify.c \
	$(LOCAL_DIR)/sd_avb_ops.c 
endif

GLOBAL_INCLUDES += $(LOCAL_DIR) \
	drivers/kunlun/crypto/silex_crypto/include \
	drivers/kunlun/crypto/silex_crypto/crypto_hal/inc/ \
	lib/libavb \
	lib/sd_x509 \
	lib/sd_crc32 \
	include/lib/verified_boot \
	include \
	include/lib/partition \
	include/lib/sd_boot_img/ \
	lib/partition/ \
	middleware/mem_image/

MODULE_CFLAGS += -Wno-strict-prototypes

include $(MKROOT)/module.mk
