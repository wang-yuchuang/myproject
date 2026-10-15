CSRCS += lv_port_indev.c

DEPPATH += --dep-path $(LVGL_SDRV_DIR_NAME)/input
VPATH += :$(LVGL_SDRV_DIR_NAME)/input
CFLAGS += "-I$(LVGL_SDRV_DIR_NAME)/input"
