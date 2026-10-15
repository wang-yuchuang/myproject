CSRCS += lv_gpu_sdrv_dma2d.c

DEPPATH += --dep-path $(LVGL_SDRV_DIR_NAME)/gpu
VPATH += :$(LVGL_SDRV_DIR_NAME)/gpu
CFLAGS += "-I$(LVGL_SDRV_DIR_NAME)/gpu"
