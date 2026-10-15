LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/common

MODULE_SRCS += \
	$(LOCAL_DIR)/common/s_matherr.c \
	$(LOCAL_DIR)/common/s_lib_ver.c \
	$(LOCAL_DIR)/math/e_log.c \
	$(LOCAL_DIR)/math/e_log10.c \
	$(LOCAL_DIR)/math/ef_log.c \
	$(LOCAL_DIR)/math/ef_log10.c \
	$(LOCAL_DIR)/math/w_log.c \
	$(LOCAL_DIR)/math/w_log10.c \
	$(LOCAL_DIR)/math/wf_log.c \
	$(LOCAL_DIR)/math/wf_log10.c

include $(MKROOT)/module.mk