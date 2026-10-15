#####################################################################
# File name: rules.mk
# Author: chentianming
# Mail:  tianming.chen@semidrive.com
# Created Time:  23/10/2019
# Copyright (C) 2019 Semidrive Technology Co.Ltd.
#####################################################################

LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(GLOBAL_INCLUDES)  \
	$(LOCAL_DIR)/vpuapi

MODULE_SRCS += \
	$(LOCAL_DIR)/vdi/nonos/vdi.c \
	$(LOCAL_DIR)/vdi/nonos/vdi_osal.c  \
	$(LOCAL_DIR)/vpuapi/product.c \
	$(LOCAL_DIR)/vpuapi/vpuapifunc.c \
	$(LOCAL_DIR)/vpuapi/vpuapi.c \
	$(LOCAL_DIR)/vpuapi/coda9/coda9.c \
	$(LOCAL_DIR)/vpuapi/wave/common/common.c \
	$(LOCAL_DIR)/vpuapi/wave/coda7q/coda7q.c \
	$(LOCAL_DIR)/vpuapi/wave/wave4/wave4.c \
	$(LOCAL_DIR)/vpuapi/wave/wave5/wave5.c

MODULE_DEFS += \

MODULE_COMPILEFLAGS += -Wno-maybe-uninitialized -Wno-int-to-pointer-cast -Wno-unused-variable

include $(MKROOT)/module.mk
