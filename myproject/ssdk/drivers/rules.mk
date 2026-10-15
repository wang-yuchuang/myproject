LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(CONFIG_ADC), y)
MODULE_SRCS += drivers/adc.c
endif

ifeq ($(CONFIG_CAN), y)
MODULE_SRCS += drivers/can.c
endif

ifeq ($(CONFIG_CLK), y)
MODULE_SRCS += drivers/clk.c
endif

ifeq ($(CONFIG_DMA), y)
MODULE_SRCS += drivers/dma.c
endif

ifeq ($(CONFIG_GPIO), y)
MODULE_SRCS += drivers/gpio.c
endif

ifeq ($(CONFIG_HWMUTEX), y)
MODULE_SRCS += drivers/hwmutex.c
endif

ifeq ($(CONFIG_I2C), y)
MODULE_SRCS += drivers/i2c.c
endif

ifeq ($(CONFIG_IRDA), y)
MODULE_SRCS += drivers/irda.c
endif

ifeq ($(CONFIG_IRQ), y)
MODULE_SRCS += drivers/irq.c
endif

ifeq ($(CONFIG_LIN), y)
MODULE_SRCS += drivers/lin.c
endif

ifeq ($(CONFIG_MBOX), y)
MODULE_SRCS += drivers/mbox.c
endif

ifeq ($(CONFIG_MEMMAP), y)
MODULE_SRCS += drivers/memmap.c
endif

ifeq ($(CONFIG_PCIE), y)
MODULE_SRCS += drivers/pcie.c
endif

ifeq ($(CONFIG_PM), y)
MODULE_SRCS += drivers/pm.c
endif

ifeq ($(CONFIG_PWM), y)
MODULE_SRCS += drivers/pwm.c
endif

ifeq ($(CONFIG_RESETGEN), y)
MODULE_SRCS += drivers/reset.c
endif

ifeq ($(CONFIG_RTC), y)
MODULE_SRCS += drivers/rtc.c
endif

ifeq ($(CONFIG_SCR), y)
MODULE_SRCS += drivers/scr.c
endif

ifeq ($(CONFIG_SENT), y)
MODULE_SRCS += drivers/sent.c
endif

ifeq ($(CONFIG_SND), y)
MODULE_SRCS += drivers/snd_driver.c
endif

ifneq (, $(findstring y, $(CONFIG_OSPI) $(CONFIG_XSPI)))
MODULE_SRCS += drivers/spi_nor.c
ifeq ($(CONFIG_HYPERBUS_MODE), y)
MODULE_SRCS += drivers/hyperbus.c
endif
endif

ifeq ($(CONFIG_SPI), y)
MODULE_SRCS += drivers/spi.c
endif

ifeq ($(CONFIG_TIMER), y)
MODULE_SRCS += drivers/timer.c \
               drivers/arch_timer.c
endif

ifeq ($(CONFIG_UART), y)
MODULE_SRCS += drivers/uart.c
endif

ifeq ($(CONFIG_WDOG), y)
MODULE_SRCS += drivers/watchdog.c
endif

ifeq ($(CONFIG_CDNS_OSPI_MST_SLV), y)
MODULE_SRCS += drivers/ospi_mst_slv.c
endif

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_E3) $(CONFIG_ARCH_CHIP_D3)))
MODULE_DEPS += drivers/taishan
endif

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_D9) $(CONFIG_ARCH_CHIP_G9) $(CONFIG_ARCH_CHIP_X9)))
MODULE_DEPS += drivers/kunlun
MODULE_DEPS += drivers/taihang
endif

ifneq (, $(findstring y, $(CONFIG_BOARD_TAISHAN_TIANJIE)))
MODULE_DEPS += drivers/xilinx
endif

