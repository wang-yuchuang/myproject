LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/sdrv_vic.c

# vic operation mode:
# (1)VIC_VECTOR_IRQ_INTERRUPT_MODE - vectored IRQ interrupt mode
# (2)VIC_VECTOR_INTERRUPT_MODE	   - vectored interrupt mode
ifeq ($(CONFIG_VIC_IRQ_INTERRUPT_MODE), y)
MODULE_ARM_OVERRIDE_SRCS := $(LOCAL_DIR)/sdrv_vic_s.S
endif

include $(MKROOT)/module.mk
