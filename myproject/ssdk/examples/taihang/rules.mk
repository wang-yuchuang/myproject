LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_APPLICATION), y)
MODULES += $(LOCAL_DIR)/application
endif

ifeq ($(CONFIG_DRIVERS_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/drivers
endif
