LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_DRIVERS_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/drivers
endif

ifeq ($(CONFIG_GUI_APPLICATION), y)
MODULES += $(LOCAL_DIR)/application
endif

ifeq ($(CONFIG_BOOT_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/boot
endif

ifeq ($(CONFIG_UTIL_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/util
endif

ifeq ($(CONFIG_OTA_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/ota
endif

ifeq ($(CONFIG_MOTOR_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/motor
endif
