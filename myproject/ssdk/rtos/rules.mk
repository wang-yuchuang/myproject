ifeq ($(CONFIG_OS_FREERTOS), y)
MODULE_DEPS += rtos/FreeRTOS
endif

ifeq ($(CONFIG_OS_BAREMETAL), y)
MODULE_DEPS += rtos/BareMetal
endif
