LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/common_utility.c \
	$(LOCAL_DIR)/sdrv_slimfs_cache.c \
	$(LOCAL_DIR)/sdrv_slimfs_checksum.c \
	$(LOCAL_DIR)/sdrv_slimfs_impl.c \
	$(LOCAL_DIR)/sdrv_slimfs_sysres_wrapper.c \
	$(LOCAL_DIR)/sdrv_slimfs_utility.c \
	$(LOCAL_DIR)/mmc/slimfs_medium_mmc.c

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \
	$(LOCAL_DIR)

include $(MKROOT)/module.mk
