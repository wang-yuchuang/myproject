LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
ifeq ($(CONFIG_CHECKSUM), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/adler32.c \
	$(LOCAL_DIR)/crc4.c \
	$(LOCAL_DIR)/crc16.c \
	$(LOCAL_DIR)/crc32.c \
	$(LOCAL_DIR)/md5.c 

endif
MODULE_CFLAGS += -Wno-strict-prototypes

include $(MKROOT)/module.mk
