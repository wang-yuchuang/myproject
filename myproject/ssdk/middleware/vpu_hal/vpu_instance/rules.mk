LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/src/vpu_instance.c \
	$(LOCAL_DIR)/proto/vpu.pb-c.c \
	$(LOCAL_DIR)/protobuf-c/protobuf-c.c \

MODULE_DEPS += \

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/ \
	$(LOCAL_DIR)/inc/ \
	$(LOCAL_DIR)/proto/ \

include $(MKROOT)/module.mk