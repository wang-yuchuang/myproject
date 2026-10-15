LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DISP_TESTER), y)
MODULE_SRCS += $(LOCAL_DIR)/dshow/buffer.c \
	$(LOCAL_DIR)/dshow/dshow.c
    ifeq ($(CONFIG_GPU_HAL), y)
        MODULE_SRCS += $(LOCAL_DIR)/dshow/g2dlite.c
    endif
endif

ifeq ($(CONFIG_DISP_PQ_TESTER), y)
MODULE_SRCS += $(LOCAL_DIR)/pq_test.c
endif

include $(MKROOT)/module.mk
