LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_ADC), y)
MODULES += $(LOCAL_DIR)/adc
endif

ifeq ($(CONFIG_AUDIO_MANAGER), y)
MODULES += $(LOCAL_DIR)/am
endif

ifeq ($(CONFIG_CAN), y)
MODULES += $(LOCAL_DIR)/can
endif

ifeq ($(CONFIG_CLK), y)
MODULES += $(LOCAL_DIR)/clk
endif

ifeq ($(CONFIG_SILEX_CRYPTO), y)
MODULES +=  $(LOCAL_DIR)/crypto/silex_crypto
endif

ifeq ($(CONFIG_CSI), y)
MODULES += $(LOCAL_DIR)/csi
endif

ifeq ($(CONFIG_DISPSS), y)
MODULES +=  $(LOCAL_DIR)/dispss
endif

ifeq ($(CONFIG_DMA), y)
MODULES += $(LOCAL_DIR)/dma
endif

ifeq ($(CONFIG_ETH), y)
MODULES += $(LOCAL_DIR)/eth
endif

ifeq ($(CONFIG_FUSE), y)
MODULES += $(LOCAL_DIR)/fuse
endif

ifeq ($(CONFIG_G2DLITE), y)
MODULES +=  $(LOCAL_DIR)/g2dlite
endif

ifeq ($(CONFIG_IRQ), y)
MODULES += $(LOCAL_DIR)/gic
endif

ifeq ($(CONFIG_GPIO), y)
MODULES += $(LOCAL_DIR)/gpio
endif

ifeq ($(CONFIG_I2C), y)
MODULES += $(LOCAL_DIR)/i2c
endif

ifeq ($(CONFIG_UART_LIN), y)
MODULES += $(LOCAL_DIR)/lin
endif

ifeq ($(CONFIG_MBOX), y)
MODULES += $(LOCAL_DIR)/mbox
endif

ifeq ($(CONFIG_PCIE), y)
MODULES += $(LOCAL_DIR)/pcie
endif

ifeq ($(CONFIG_PINCTRL), y)
MODULES += $(LOCAL_DIR)/pinctrl
endif

ifeq ($(CONFIG_PMU), y)
MODULES += $(LOCAL_DIR)/pmu
endif

ifeq ($(CONFIG_PVT), y)
MODULES += $(LOCAL_DIR)/pvt
endif

ifeq ($(CONFIG_PWM), y)
MODULES += $(LOCAL_DIR)/pwm
endif

ifeq ($(CONFIG_RESETGEN), y)
MODULES += $(LOCAL_DIR)/reset
endif

ifeq ($(CONFIG_RTC), y)
MODULES += $(LOCAL_DIR)/rtc
endif

ifeq ($(CONFIG_SCR), y)
MODULES += $(LOCAL_DIR)/scr
endif

ifeq ($(CONFIG_SEM), y)
MODULES += $(LOCAL_DIR)/sem
MODULES += $(LOCAL_DIR)/eic
endif

ifeq ($(CONFIG_SERDES), y)
MODULES += $(LOCAL_DIR)/serdes
endif

ifeq ($(CONFIG_SND), y)
MODULES += $(LOCAL_DIR)/snd
endif

ifeq ($(CONFIG_SPI), y)
MODULES += $(LOCAL_DIR)/spi
endif

ifeq ($(CONFIG_OSPI), y)
MODULES += $(LOCAL_DIR)/spi_nor
endif

ifeq ($(CONFIG_SDRV_SYSCNT), y)
MODULES += $(LOCAL_DIR)/syscnt
endif

ifeq ($(CONFIG_SDRV_TMR), y)
MODULES += $(LOCAL_DIR)/tmr
endif

ifeq ($(CONFIG_TOUCH), y)
MODULES += $(LOCAL_DIR)/touch
endif

ifeq ($(CONFIG_UART), y)
MODULES += $(LOCAL_DIR)/uart
endif

ifeq ($(CONFIG_SDRV_VIRT_UART), y)
MODULES += $(LOCAL_DIR)/virt_uart
endif

ifeq ($(CONFIG_SUPPORT_LOG_TIMESTAMP), y)
MODULES += $(LOCAL_DIR)/timestamp
endif

ifeq ($(CONFIG_UNILINK), y)
MODULES += $(LOCAL_DIR)/ulink
endif

ifeq ($(CONFIG_SDRV_KUNLUN_VPU), y)
MODULES += $(LOCAL_DIR)/vpu
endif

ifeq ($(CONFIG_VPU_DEV_API), y)
MODULE_DEPS += $(LOCAL_DIR)/vpu_dev_api
endif

ifeq ($(CONFIG_WDOG), y)
MODULES += $(LOCAL_DIR)/watchdog
endif

ifeq ($(CONFIG_KUNLUN_MMC), y)
MODULE_DEPS += $(LOCAL_DIR)/mmc
endif

ifeq ($(CONFIG_PFM), y)
MODULE_DEPS += $(LOCAL_DIR)/pfm
endif

ifeq ($(CONFIG_PMIC), y)
MODULE_DEPS += $(LOCAL_DIR)/pmic
endif
