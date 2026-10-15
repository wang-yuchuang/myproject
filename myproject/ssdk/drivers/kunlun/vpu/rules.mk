LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

VMEM_PAGE_SIZE ?= 0x100000

GLOBAL_DEFINES += \
	VPU_CODAJ12_DRV_MEM_SIZE=$(VPU_CODAJ12_DRV_MEM_SIZE) \
	VMEM_PAGE_SIZE=$(VMEM_PAGE_SIZE)


GLOBAL_INCLUDES += \
    $(GLOBAL_INCLUDES)  \
    $(LOCAL_DIR)/codaj12/inc  \
    $(LOCAL_DIR)/codaj12/inc/jpuapi   \
    $(LOCAL_DIR)/codaj12/inc/jdi

MODULE_SRCS += \
	$(LOCAL_DIR)/codaj12/src/jpuapi/jpulog.c \
	$(LOCAL_DIR)/codaj12/src/jdi/jdi.c  \
	$(LOCAL_DIR)/codaj12/src/jpuapi/jpuapi.c \
	$(LOCAL_DIR)/codaj12/src/jpuapi/jpuapifunc.c \
    $(LOCAL_DIR)/codaj12/src/jdi/mm.c \
	$(LOCAL_DIR)/sdrv_jpu.c \
	$(LOCAL_DIR)/vpu.c

MODULE_DEFS += \
    lib/cbuf

# include make/module.mk