CAN_PROXY_DIR := $(GET_LOCAL_DIR)

MODULE := $(CAN_PROXY_DIR)

GLOBAL_INCLUDES += \
		$(CAN_PROXY_DIR)/include \
		$(CAN_PROXY_DIR)/can/flexcan/include

ifeq ($(CONFIG_VCAN_CLIENT), y)
	MODULE_SRCS += $(CAN_PROXY_DIR)/vcan_client.c
else
	include $(CAN_PROXY_DIR)/can/rules.mk

	MODULE_SRCS += \
		$(CAN_PROXY_DIR)/can_proxy.c \
		$(CAN_PROXY_DIR)/bundle/can_proxy_bundle.c \
		$(CAN_PROXY_DIR)/interface/can_proxy_if.c \
		$(CAN_PROXY_DIR)/interface/can/can_port.c \
		$(CAN_PROXY_DIR)/interface/rpmsg/rpmsg_port.c \
		$(CAN_PROXY_DIR)/utils/can_proxy_buffer.c \
		$(CAN_PROXY_DIR)/utils/can_proxy_os_port.c
endif

MODULE_COMPILEFLAGS += -Wall

include $(MKROOT)/module.mk