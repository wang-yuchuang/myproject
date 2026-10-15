LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_INCLUDES += $(LOCAL_DIR)

MODULE_COMPILEFLAGS += -Wall -Werror

# TODO debug option
SHARE_MEMORY_DEBUG := true

# TODO where is server deployed (secure for now)
SHARE_MEMORY_SERVER_RPROC := 1

MODULE_SRCS += $(LOCAL_DIR)/shm_manager.c
MODULE_SRCS += $(LOCAL_DIR)/shm_client.c

ifeq ($(SHARE_MEMORY_DEBUG),true)
MODULE_CFLAGS += -D__SHARE_MEMORY_DEBUG
endif

MODULE_CFLAGS += -D__SERVER_RPROC=$(SHARE_MEMORY_SERVER_RPROC)

include $(MKROOT)/module.mk

