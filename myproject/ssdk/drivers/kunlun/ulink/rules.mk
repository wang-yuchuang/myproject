LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/ulink_main.c \
	$(LOCAL_DIR)/ulink_irq.c \
	$(LOCAL_DIR)/ulink_channel.c 
#	$(LOCAL_DIR)/ulink_test.c

ifeq ($(CONFIG_LWIP),y)
MODULE_SRCS += $(LOCAL_DIR)/ulink_net.c

endif


MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast


ifneq ($(findstring $(ULINK_HPL_DMA_SERVICE_TYPE),server client),)
# service type
ifeq ($(ULINK_HPL_DMA_SERVICE_TYPE),server)
GLOBAL_DEFINES += ULINK_HPL_DMA_COPY_SERVER=1
else
GLOBAL_DEFINES += ULINK_HPL_DMA_COPY_SERVER=0
endif

# service address
ifneq ($(ULINK_HPL_DMA_SERVICE_ADDR),)
GLOBAL_DEFINES += ULINK_HPL_DMA_COPY_SERVER_ADDR=\"$(ULINK_HPL_DMA_SERVICE_ADDR)\"
else
GLOBAL_DEFINES += ULINK_HPL_DMA_COPY_SERVER_ADDR=\"172.20.2.38\"
endif

# service port
ifneq ($(ULINK_HPL_DMA_SERVICE_PORT),)
GLOBAL_DEFINES += ULINK_HPL_DMA_COPY_SERVER_PORT=$(ULINK_HPL_DMA_SERVICE_PORT)
else
GLOBAL_DEFINES += ULINK_HPL_DMA_COPY_SERVER_PORT=21843
endif

GLOBAL_INCLUDES += middleware/lwip/src/include/compat/posix
MODULE_SRCS += $(LOCAL_DIR)/ulink_hpl_dma_service.c
MODULE_DEPS += middleware/lwip
endif

include mk/module.mk
