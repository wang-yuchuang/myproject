LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
		$(LOCAL_DIR)/vpu_test.c

MODULE_DEPS += \
	lib/res_loader \

MODULE_COMPILEFLAGS += -Wno-unused-but-set-variable -Wno-enum-compare

include $(MKROOT)/module.mk
