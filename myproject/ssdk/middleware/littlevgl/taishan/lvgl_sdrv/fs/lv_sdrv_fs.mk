CSRCS += fatfs_semidrive.c

DEPPATH += --dep-path $(LVGL_SDRV_DIR_NAME)/fs
VPATH += :$(LVGL_SDRV_DIR_NAME)/fs
CFLAGS += "-I$(LVGL_SDRV_DIR_NAME)/fs"
