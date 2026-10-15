LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_USB_DEMO), y)
MODULES += $(LOCAL_DIR)/usb
endif

ifeq ($(CONFIG_DMA_DEMO), y)
MODULES += $(LOCAL_DIR)/dma
endif

ifeq ($(CONFIG_SENT_DEMO), y)
MODULES += $(LOCAL_DIR)/sent
endif

ifeq ($(CONFIG_UART_DEMO), y)
MODULES += $(LOCAL_DIR)/uart
endif

ifeq ($(CONFIG_LIN_DEMO), y)
MODULES += $(LOCAL_DIR)/lin
endif

ifeq ($(CONFIG_ADC_DEMO), y)
MODULES += $(LOCAL_DIR)/adc
endif

ifeq ($(CONFIG_MBOX_DEMO), y)
MODULES += $(LOCAL_DIR)/mbox
endif

ifeq ($(CONFIG_RPMSG_DEMO), y)
MODULES += $(LOCAL_DIR)/rpmsg
endif

ifeq ($(CONFIG_CANFD_DEMO), y)
MODULES += $(LOCAL_DIR)/canfd
endif

ifeq ($(CONFIG_FUSE_UTILITY), y)
MODULES += $(LOCAL_DIR)/fuse
endif

ifeq ($(CONFIG_LED_DEMO), y)
MODULES += $(LOCAL_DIR)/led
endif

ifeq ($(CONFIG_ANALOG_IN_DEMO), y)
MODULES += $(LOCAL_DIR)/analog_in
endif

ifeq ($(CONFIG_ANALOG_OUT_DEMO), y)
MODULES += $(LOCAL_DIR)/analog_out
endif

ifeq ($(CONFIG_HSD_DEMO), y)
MODULES += $(LOCAL_DIR)/hsd
endif

ifeq ($(CONFIG_ACMP_DEMO), y)
MODULES += $(LOCAL_DIR)/acmp
endif

ifeq ($(CONFIG_LSD_DEMO), y)
MODULES += $(LOCAL_DIR)/lsd
endif

ifeq ($(CONFIG_SBC), y)
MODULES += $(LOCAL_DIR)/sbc
endif

ifeq ($(CONFIG_PMU_DEMO), y)
MODULES += $(LOCAL_DIR)/pmu
endif

ifeq ($(CONFIG_WDOG_DEMO), y)
MODULES += $(LOCAL_DIR)/watchdog
endif

ifeq ($(CONFIG_SDRV_TMR_DEMO), y)
MODULES += $(LOCAL_DIR)/timer
endif

ifeq ($(CONFIG_DISK_DEMO), y)
MODULES += $(LOCAL_DIR)/disk
endif

ifeq ($(CONFIG_FEE_DEMO), y)
MODULES += $(LOCAL_DIR)/fee
endif

ifeq ($(CONFIG_PINCTRL_DEMO), y)
MODULE_DEPS += $(LOCAL_DIR)/pinctrl
endif

ifeq ($(CONFIG_CPUFREQ_DEMO), y)
MODULE_DEPS += $(LOCAL_DIR)/cpufreq
endif

ifeq ($(CONFIG_I2C_DEMO), y)
MODULES += $(LOCAL_DIR)/i2c
endif

ifeq ($(CONFIG_ETH_DEMO), y)
MODULES += $(LOCAL_DIR)/eth
endif

ifeq ($(CONFIG_POWER_DEMO), y)
MODULES += $(LOCAL_DIR)/power
endif

ifeq ($(CONFIG_TAISHAN_SPI_DEMO), y)
MODULES += $(LOCAL_DIR)/spi
endif

ifeq ($(CONFIG_PWM_DEMO), y)
MODULES += $(LOCAL_DIR)/pwm
endif

ifeq ($(CONFIG_RTC_DEMO), y)
MODULES += $(LOCAL_DIR)/rtc
endif
