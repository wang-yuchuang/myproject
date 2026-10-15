LCM_DIR := $(GET_LOCAL_DIR)

MODULE := $(LCM_DIR)

GLOBAL_INCLUDES += $(LCM_DIR)

ifeq ($(CONFIG_LVDS_HSD123_SERDES_1920x720_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_hsd123_serdes_1920x720_lcd.c
endif

ifeq ($(CONFIG_MIPI_HSD123_SERDES_1920x720_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/mipi_hsd123_serdes_1920x720_lcd.c
endif

ifeq ($(CONFIG_MIPI_HSD123_SERDES_1920x720_2_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/mipi_hsd123_serdes_1920x720_2_lcd.c
endif

ifeq ($(CONFIG_LVDS_HSD101_SERDES_1280x800_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_hsd101_serdes_1280x800_lcd.c
endif

ifeq ($(CONFIG_LVDS_HSD123_SERDES_1920x720_x9u_b_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_hsd123_serdes_1920x720_x9u_b_lcd.c
endif

ifeq ($(CONFIG_LVDS_KD070_SERDES_1024x600_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_kd070_serdes_1024x600_lcd.c
endif

ifeq ($(LVDS_LMT070_SERDES_800x480_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_lmt070_serdes_800x480_lcd.c
endif

ifeq ($(CONFIG_LVDS_WALLFACERA2_HSD123_SERDES_1920x720_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_WallfacerA2_hsd123_serdes_1920x720_lcd.c
endif

ifeq ($(CONFIG_LVDS_WALLFACERA2_HSD123_SERDES_1920x720_2_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_WallfacerA2_hsd123_serdes_1920x720_2_lcd.c
endif

ifeq ($(CONFIG_MIPI_WALLFACERA2_HSD123_SERDES_1920x720_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/mipi_WallfacerA2_hsd123_serdes_1920x720_lcd.c
endif

ifeq ($(CONFIG_MIPI_WALLFACERA2_HSD123_SERDES_1920x720_2_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/mipi_WallfacerA2_hsd123_serdes_1920x720_2_lcd.c
endif

ifeq ($(CONFIG_MIPI_WALLFACERA_HSD160_SERDES_2560x1600_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/mipi_WallfacerA_hsd160_serdes_2560x1600_lcd.c
endif

ifeq ($(CONFIG_LVDS_KD070_MAX_SERDES_1024x600_LCD), y)
MODULE_SRCS += \
	$(LCM_DIR)/lvds_kd070_max_serdes_1024x600_lcd.c
endif

MODULE_SRCS += \
	$(LCM_DIR)/disp_panels.c

ifeq ($(CONFIG_DS90UB9XX), y)
include $(LCM_DIR)/ds90ub9xx/rules.mk
endif

ifeq ($(CONFIG_MAXIM_DISP), y)
include $(LCM_DIR)/maxim/rules.mk
endif
