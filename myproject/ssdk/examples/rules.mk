LOCAL_DIR := $(GET_LOCAL_DIR)

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_E3) $(CONFIG_ARCH_CHIP_D3)))
MODULE_DEPS += examples/taishan
endif

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_D9) $(CONFIG_ARCH_CHIP_G9) $(CONFIG_ARCH_CHIP_X9)))
MODULE_DEPS += examples/kunlun
endif

ifneq (, $(findstring y, $(CONFIG_ARCH_CHIP_X9)))
MODULE_DEPS += examples/taihang
endif

ifneq ($(wildcard examples/bms/Functions/),)
MODULE_DEPS += examples/bms
GLOBAL_CFLAGS += -DCONFIG_BMS_MODULE
endif