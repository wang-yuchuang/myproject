MODULE_SRCS += $(LVGL_SDRV_DIR_NAME)/lvgl_gui.c

ifeq ($(CONFIG_LVGL_SDRV_DISPLAY), y)
include $(LVGL_SDRV_DIR_NAME)/display/lv_sdrv_display.mk
include $(LVGL_DIR)/convert_lk.mk
endif

ifeq ($(CONFIG_LVGL_SDRV_DECODER_PNG), y)
include $(LVGL_SDRV_DIR_NAME)/decoder/lodepng/lv_sdrv_png.mk
include $(LVGL_DIR)/convert_lk.mk
endif

ifeq ($(CONFIG_LVGL_SDRV_DECODER_JPG), y)
include $(LVGL_SDRV_DIR_NAME)/decoder/sjpg/lv_sdrv_sjpg.mk
include $(LVGL_DIR)/convert_lk.mk
endif

ifeq ($(CONFIG_LVGL_SDRV_FAT_FS), y)
include $(LVGL_SDRV_DIR_NAME)/fs/lv_sdrv_fs.mk
include $(LVGL_DIR)/convert_lk.mk
endif

ifeq ($(CONFIG_LVGL_SDRV_GPU), y)
include $(LVGL_SDRV_DIR_NAME)/gpu/lv_sdrv_gpu.mk
include $(LVGL_DIR)/convert_lk.mk
endif

ifeq ($(CONFIG_LVGL_SDRV_INPUT), y)
include $(LVGL_SDRV_DIR_NAME)/input/lv_sdrv_input.mk
include $(LVGL_DIR)/convert_lk.mk
endif
