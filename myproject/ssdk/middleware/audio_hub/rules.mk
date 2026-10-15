LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_DEPS += $(LOCAL_DIR)


GLOBAL_INCLUDES := \
	$(GLOBAL_INCLUDES) \
	$(LOCAL_DIR)/inc/api


#MODULE_SRCS += 	$(LOCAL_DIR)/src/ahub_debug.c \


EXTRA_OBJS += $(LOCAL_DIR)/lib/libahub_lib.a
#MODULE_CFLAGS += -Wno-strict-prototypes -Wno-unused-variable -Wall -E

include $(MKROOT)/module.mk
