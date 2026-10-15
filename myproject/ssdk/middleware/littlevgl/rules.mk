LOCAL_DIR := $(GET_LOCAL_DIR)
LVGL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
LVGL_DIR_NAME := lvgl-release-v7

GLOBAL_INCLUDES += \
	$(LVGL_DIR) \
	$(LVGL_DIR)/$(LVGL_DIR_NAME) \

include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_E3) $(CONFIG_ARCH_CHIP_D3)))
MODULE_DEPS += middleware/littlevgl/taishan
endif

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_D9) $(CONFIG_ARCH_CHIP_G9) $(CONFIG_ARCH_CHIP_X9)))
MODULE_DEPS += middleware/littlevgl/kunlun
endif

include $(MKROOT)/module.mk
