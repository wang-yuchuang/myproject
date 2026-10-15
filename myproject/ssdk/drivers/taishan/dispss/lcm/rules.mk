LCM_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_LVDS_800X600_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_800x600_lcd.c
endif

ifeq ($(CONFIG_HDMI_ADV_800X600_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/hdmi_adv_800x600_lcd.c
endif

ifeq ($(CONFIG_RGB_1280X800_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/rgb_1280x720_lcd.c
endif

ifeq ($(CONFIG_RGB_800X600_LCD_Z1), y)
MODULE_SRCS += \
	$(LCM_DIR)/rgb_800x600_lcd_z1.c
endif

ifeq ($(CONFIG_LVDS_1440X540_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_1440x540_lcd.c
endif

ifeq ($(CONFIG_LVDS_1280X768_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_1280x768_lcd.c
endif

ifeq ($(CONFIG_RGB_480X272_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/rgb_480x272_lcd.c
endif

ifeq ($(CONFIG_LVDS_LT9211_TO_MIPI_480x1920_LCD), y)
MODULE_SRCS += \
        $(LCM_DIR)/lvds_lt9211_to_mipi_480x1920_lcd.c \
	$(LCM_DIR)/lt9211/lt9211.c
endif

ifeq ($(CONFIG_LVDS_1920X1080_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_1920x1080_lcd.c
endif

ifeq ($(CONFIG_LVDS_LT9211_1PORT_TO_2PORT_1920x720_LCD), y)
MODULE_SRCS += \
        $(LCM_DIR)/lvds_lt9211_1port_to_2port_1920x720_lcd.c \
	$(LCM_DIR)/lvds_lt9211_1port_to_2port/lvds_lt9211_1port_to_2port.c
endif
