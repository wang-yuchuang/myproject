LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_BOARD_X9SP), y)
GLOBAL_INCLUDES += $(LOCAL_DIR)/src/res/thang
GLOBAL_CFLAGS += -DIS_THANG=1
GLOBAL_CFLAGS += -DMODULE_HELPER_CKGEN_NNA=0
else
GLOBAL_INCLUDES += $(LOCAL_DIR)/src/res/kunlun
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc
MODULE_SRCS += \
	$(LOCAL_DIR)/src/module_helper_hal.c \
	$(LOCAL_DIR)/src/res/res_clk.c	\
	$(LOCAL_DIR)/src/res/res_rstgen.c	\

ifeq ($(CONFIG_MODULE_HELPER_PER_DDR), y)
MODULE_SRCS +=	$(LOCAL_DIR)/src/per/per_ddr.c
endif

ifeq ($(CONFIG_MODULE_HELPER_PER_SYS), y)
MODULE_SRCS +=	$(LOCAL_DIR)/src/per/per_sys.c
endif

ifeq ($(CONFIG_MODULE_HELPER_PER_TEST), y)
MODULE_SRCS +=	$(LOCAL_DIR)/src/per/per_test.c
endif

ifeq ($(CONFIG_MODULE_HELPER_PER_DISP), y)
ifeq ($(CONFIG_BOARD_X9SP), y)
MODULE_SRCS +=  $(LOCAL_DIR)/src/per/thang/per_disp.c
else
MODULE_SRCS +=  $(LOCAL_DIR)/src/per/per_disp.c
endif
endif

include $(MKROOT)/module.mk
