CSRCS += lvgl_disp_drv.c

DEPPATH += --dep-path $(LVGL_SDRV_DIR_NAME)/display
VPATH += :$(LVGL_SDRV_DIR_NAME)/display
CFLAGS += "-I$(LVGL_SDRV_DIR_NAME)/display"
