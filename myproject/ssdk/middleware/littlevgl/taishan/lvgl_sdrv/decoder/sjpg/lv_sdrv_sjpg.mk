CSRCS += lv_sjpg.c

DEPPATH += --dep-path $(LVGL_SDRV_DIR_NAME)/decoder/sjpg
VPATH += :$(LVGL_SDRV_DIR_NAME)/decoder/sjpg
CFLAGS += "-I$(LVGL_SDRV_DIR_NAME)/decoder/sjpg"
