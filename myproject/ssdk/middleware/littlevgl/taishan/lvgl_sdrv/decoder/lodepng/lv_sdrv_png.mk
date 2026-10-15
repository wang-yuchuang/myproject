CSRCS += lv_png.c

DEPPATH += --dep-path $(LVGL_SDRV_DIR_NAME)/decoder/lodepng
VPATH += :$(LVGL_SDRV_DIR_NAME)/decoder/lodepng
CFLAGS += "-I$(LVGL_SDRV_DIR_NAME)/decoder/lodepng"
