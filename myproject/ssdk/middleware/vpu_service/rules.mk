LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/vpu_svc_cmd.c \
	$(LOCAL_DIR)/vpu_virtio_svc.c

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/ \

#MODULE_DEPS += framework/communication
#MODULE_DEPS += framework/service/virtio

MODULE_COMPILEFLAGS += -Wno-int-to-pointer-cast -Wno-incompatible-pointer-types

include $(MKROOT)/module.mk
