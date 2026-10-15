LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_APP_EMIRROR), y)
MODULES += $(LOCAL_DIR)/emirror
endif

ifeq ($(CONFIG_APP_CLUSTER), y)
MODULES += $(LOCAL_DIR)/cluster
endif

ifeq ($(CONFIG_APP_CONTROLPANEL), y)
MODULES += $(LOCAL_DIR)/controlpanel
endif

ifeq ($(CONFIG_APP_QT_CLUSTER), y)
MODULES += $(LOCAL_DIR)/qt_app
endif

ifeq ($(CONFIG_APP_MJPEG_DEC), y)
MODULES += $(LOCAL_DIR)/mjpeg_dec
endif

ifeq ($(CONFIG_APP_MP3_DEC), y)
MODULES += $(LOCAL_DIR)/mp3_dec
endif

ifeq ($(CONFIG_APP_BOOTANIMATION), y)
MODULES += $(LOCAL_DIR)/bootanimation
endif

ifeq ($(CONFIG_APP_LAUNCHER), y)
MODULES += $(LOCAL_DIR)/launcher
endif

ifeq ($(CONFIG_APP_CAM_PRE), y)
MODULE_DEPS += $(LOCAL_DIR)/cam_pre
endif