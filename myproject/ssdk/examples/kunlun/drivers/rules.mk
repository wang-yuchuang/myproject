LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_BACKLIGHT_DEMO), y)
MODULES += $(LOCAL_DIR)/backlight
endif

ifeq ($(CONFIG_SILEX_CRYPTO_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/crypto/crypto_silex
endif

ifeq ($(CONFIG_DISP_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/disp
endif

ifeq ($(CONFIG_FUSE_DEMO), y)
MODULES += $(LOCAL_DIR)/fuse
endif

ifeq ($(CONFIG_GPIO_DEMO), y)
MODULES += $(LOCAL_DIR)/gpio
endif

ifeq ($(CONFIG_I2CTOOLS), y)
MODULES += $(LOCAL_DIR)/i2c
endif

ifeq ($(CONFIG_INPUT_DEMO), y)
MODULE_DEPS += $(LOCAL_DIR)/input
endif

ifeq ($(CONFIG_MBOX_DEMO), y)
MODULES += $(LOCAL_DIR)/mbox
endif

ifeq ($(CONFIG_PMU_DEMO), y)
MODULES += $(LOCAL_DIR)/pmu
endif

ifeq ($(CONFIG_PVT_DEMO), y)
MODULES += $(LOCAL_DIR)/pvt
endif

ifeq ($(CONFIG_RPMSG_DEMO), y)
MODULES += $(LOCAL_DIR)/rpmsg
endif

ifeq ($(CONFIG_RTC_DEMO), y)
MODULES += $(LOCAL_DIR)/rtc
endif

ifeq ($(CONFIG_SERDES_DEMO), y)
MODULE_DEPS += $(LOCAL_DIR)/serdes
endif

ifeq ($(CONFIG_SPI_DEMO), y)
MODULES += $(LOCAL_DIR)/spi
endif

ifeq ($(CONFIG_TOUCH_DEMO), y)
MODULE_DEPS += $(LOCAL_DIR)/touch
endif

ifeq ($(CONFIG_VPU_DEMO), y)
MODULE_DEPS += $(LOCAL_DIR)/vpu
endif

ifeq ($(CONFIG_IRQ_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/irq
endif
ifeq ($(CONFIG_I2C_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/i2c
endif

ifeq ($(CONFIG_DMA_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/dma
endif
ifeq ($(CONFIG_CAN_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/can
endif

ifeq ($(CONFIG_PIN_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/pin
endif

ifeq ($(CONFIG_WDOG_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/watchdog
endif

ifeq ($(CONFIG_NORFLASH_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/norflash
endif

ifeq ($(CONFIG_PCIE_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/pcie
endif

ifeq ($(CONFIG_MMC_TESTER), y)
MODULE_DEPS += $(LOCAL_DIR)/mmc
endif

ifeq ($(CONFIG_DISK_DEMO), y)
MODULES += $(LOCAL_DIR)/disk
endif

ifeq ($(CONFIG_FEE_DEMO), y)
MODULES += $(LOCAL_DIR)/fee
endif

ifeq ($(CONFIG_VPU_TEST), y)
MODULES += $(LOCAL_DIR)/vpu_test
endif

ifeq ($(CONFIG_SPINLOCK_TEST), y)
MODULES += $(LOCAL_DIR)/spinlock
endif

ifeq ($(CONFIG_IPI_TEST), y)
MODULES += $(LOCAL_DIR)/ipi
endif

ifeq ($(CONFIG_ETH_TEST), y)
MODULES += $(LOCAL_DIR)/eth
endif

ifeq ($(CONFIG_DW_UART_TEST), y)
MODULES += $(LOCAL_DIR)/dw_uart
endif
