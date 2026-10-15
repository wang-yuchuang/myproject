LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_PROP_CLIENT),y)
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/client
MODULE_SRCS += \
	$(LOCAL_DIR)/client/sdrv_property.c \
	$(LOCAL_DIR)/client/sdrv_property_list.c \
	$(LOCAL_DIR)/client/sdrv_property_trans.c
else ifeq ($(CONFIG_SDRV_PROP_SERVER),y)
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/server
ifeq ($(CONFIG_SLIMFS), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/server/sdrv_property_emmc.c
endif #ifeq ($(CONFIG_SLIMFS), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/server/sdrv_property_store.c \
	$(LOCAL_DIR)/server/sdrv_property_list.c \
	$(LOCAL_DIR)/server/sdrv_property.c \
	$(LOCAL_DIR)/server/sdrv_property_trans.c
else #ifeq ($(CONFIG_SDRV_PROP_CLIENT),y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_property_weak.c
endif #ifeq ($(CONFIG_SDRV_PROP_CLIENT),y)

MODULE_COMPILEFLAGS += -Wall

include $(MKROOT)/module.mk
