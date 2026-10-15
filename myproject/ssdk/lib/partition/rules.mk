LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_PARTITION), y)

MODULE_SRCS += \
	$(LOCAL_DIR)/partition_parser.c  \
	$(LOCAL_DIR)/ab_partition_parser.c  \
	$(LOCAL_DIR)/partition_disk_io.c

endif
include $(MKROOT)/module.mk
