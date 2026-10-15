LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_SDRV_ACMP), y)
MODULES += $(LOCAL_DIR)/acmp
endif

ifeq ($(CONFIG_ADC), y)
MODULES += $(LOCAL_DIR)/adc
endif

ifeq ($(CONFIG_ASW), y)
MODULES += $(LOCAL_DIR)/asw
endif

ifeq ($(CONFIG_SDRV_BTM), y)
MODULES += $(LOCAL_DIR)/btm
endif

ifeq ($(CONFIG_CAN), y)
MODULES += $(LOCAL_DIR)/can
endif

ifeq ($(CONFIG_CLK), y)
MODULES += $(LOCAL_DIR)/clk
endif

ifeq ($(CONFIG_OSR_CRYPTO), y)
MODULES += $(LOCAL_DIR)/crypto/osr_crypto
endif

ifeq ($(CONFIG_CSI), y)
MODULES += $(LOCAL_DIR)/csi
endif

ifeq ($(CONFIG_DISPSS), y)
MODULES += $(LOCAL_DIR)/dispss
endif

ifeq ($(CONFIG_DMA), y)
MODULES += $(LOCAL_DIR)/dma
endif

ifeq ($(CONFIG_EIC), y)
MODULES += $(LOCAL_DIR)/eic
endif

ifeq ($(CONFIG_SDRV_EPWM), y)
MODULES += $(LOCAL_DIR)/epwm
endif

ifeq ($(CONFIG_ETH), y)
MODULES += $(LOCAL_DIR)/eth
endif

ifeq ($(CONFIG_SDRV_ETIMER), y)
MODULES += $(LOCAL_DIR)/etimer
endif

ifeq ($(CONFIG_FUSE), y)
MODULES += $(LOCAL_DIR)/fuse
endif

ifeq ($(CONFIG_G2DLITE), y)
MODULES += $(LOCAL_DIR)/g2dlite
endif

ifeq ($(CONFIG_GPIO), y)
MODULES += $(LOCAL_DIR)/gpio
endif

ifeq ($(CONFIG_HWMUTEX), y)
MODULES += $(LOCAL_DIR)/hwmutex
endif

ifeq ($(CONFIG_I2C), y)
MODULES += $(LOCAL_DIR)/i2c
endif

ifeq ($(CONFIG_IRDA), y)
MODULES += $(LOCAL_DIR)/irda
endif

ifeq ($(CONFIG_LIN), y)
MODULES += $(LOCAL_DIR)/lin
endif

ifeq ($(CONFIG_MBOX), y)
MODULES += $(LOCAL_DIR)/mbox
endif

ifeq ($(CONFIG_MMC), y)
MODULES += $(LOCAL_DIR)/mmc
endif

ifeq ($(CONFIG_PINCTRL), y)
MODULES += $(LOCAL_DIR)/pinctrl
endif

ifeq ($(CONFIG_PMU), y)
MODULES += $(LOCAL_DIR)/pmu
endif

ifeq ($(CONFIG_PM), y)
MODULES += $(LOCAL_DIR)/power
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

ifeq ($(CONFIG_SDRAMC), y)
MODULES += $(LOCAL_DIR)/sdramc
endif

ifeq ($(CONFIG_SEM), y)
MODULES += $(LOCAL_DIR)/sem
endif

ifeq ($(CONFIG_SENSOR), y)
MODULES += $(LOCAL_DIR)/sensor
endif

ifeq ($(CONFIG_SENT), y)
MODULES += $(LOCAL_DIR)/sent
endif

ifeq ($(CONFIG_SMC), y)
MODULES += $(LOCAL_DIR)/smc
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

ifeq ($(CONFIG_TAMPER_MONITOR), y)
MODULES += $(LOCAL_DIR)/tmonitor
endif

ifeq ($(CONFIG_TOUCH), y)
MODULES += $(LOCAL_DIR)/touch
endif

ifeq ($(CONFIG_SDRV_UART), y)
MODULES += $(LOCAL_DIR)/uart
endif

ifeq ($(CONFIG_IRQ), y)
MODULES += $(LOCAL_DIR)/vic
endif

ifeq ($(CONFIG_WDOG), y)
MODULES += $(LOCAL_DIR)/watchdog
endif

ifneq (, $(findstring y, $(CONFIG_XSPI_MST) $(CONFIG_XSPI_SLV)))
MODULES += $(LOCAL_DIR)/xspi_mst_slv
endif

ifeq ($(CONFIG_XTRG), y)
MODULES += $(LOCAL_DIR)/xtrg
endif