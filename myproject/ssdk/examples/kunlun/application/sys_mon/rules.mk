LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \
	$(LOCAL_DIR)/../../../../rtos/FreeRTOS/Source/portable/GCC/ARM_CR5 \
	$(LOCAL_DIR)/../../../../rtos/FreeRTOS/Source/include \
	$(LOCAL_DIR)/../../../../rtos/FreeRTOS/Config \
	$(LOCAL_DIR)/../../../../rtos/FreeRTOS/Source/portable/GCC/ARM_CM3 \
	$(LOCAL_DIR)/../../../../rtos/FreeRTOS/Source/portable/MemMang

MODULE_SRCS += \
	$(LOCAL_DIR)/src/sys_mon.c \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
