LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += $(LOCAL_DIR)/infra/sdpe_rpc_device.c \
               $(LOCAL_DIR)/infra/sdpe_rpc_pack.c \
               $(LOCAL_DIR)/service/sdpe_rpc_service.c

ifeq ($(CONFIG_SDPE_RPC_MBOX_TRANSPORT), y)
MODULE_SRCS += $(LOCAL_DIR)/transport/sdpe_rpc_mbox_transport.c
endif

ifeq ($(CONFIG_SDPE_RPC_RPMSG_TRANSPORT), y)
MODULE_SRCS += $(LOCAL_DIR)/transport/sdpe_rpc_rpmsg_transport.c
endif

ifeq ($(CONFIG_SDPE_RPC_SERVER), n)
MODULE_SRCS += $(LOCAL_DIR)/service/sdpe_ctrl/sdpe_rpc_ctrl_server.c \
               $(LOCAL_DIR)/service/vcan/sdpe_rpc_vcan_server.c \
               $(LOCAL_DIR)/service/viln/sdpe_rpc_vlin_server.c
else
MODULE_SRCS += $(LOCAL_DIR)/service/sdpe_ctrl/sdpe_rpc_ctrl_client.c \
               $(LOCAL_DIR)/service/vcan/sdpe_rpc_vcan_client.c \
               $(LOCAL_DIR)/service/vlin/sdpe_rpc_vlin_client.c
endif

# gcc specialized scripts
ifeq ($(SSDK_TOOLCHAIN),gcc)
GLOBAL_LDFLAGS += \
    --defsym SDPE_RPC_BUFFER_BASE=$(CONFIG_SDPE_RPC_MEM_BASE)+$(CONFIG_SDPE_RPC_MEM_OFFSET) \
    --defsym SDPE_RPC_BUFFER_SIZE=$(CONFIG_SDPE_RPC_MEM_SIZE)-$(CONFIG_SDPE_RPC_MEM_OFFSET)
EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/sdpe_rpc.ld
endif

ifeq ($(SSDK_TOOLCHAIN),iar)
SDPE_RPC_BUFFER_BASE=$(shell echo $$(($(CONFIG_SDPE_RPC_MEM_BASE)+$(CONFIG_SDPE_RPC_MEM_OFFSET))))
SDPE_RPC_BUFFER_SIZE=$(shell echo $$(($(CONFIG_SDPE_RPC_MEM_SIZE)-$(CONFIG_SDPE_RPC_MEM_OFFSET))))
IAR_LINKER_OPTIONS += \
    --config_def SDPE_RPC_BUFFER_BASE=$(SDPE_RPC_BUFFER_BASE) \
    --config_def SDPE_RPC_BUFFER_SIZE=$(SDPE_RPC_BUFFER_SIZE)
endif

include $(MKROOT)/module.mk
