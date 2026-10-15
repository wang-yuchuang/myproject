ifeq ($(CONFIG_BUFFQUEUE), y)
MODULE_DEPS += lib/buffqueue
endif

ifeq ($(CONFIG_CHECKSUM), y)
MODULE_DEPS += lib/checksum
endif

ifeq ($(CONFIG_SD_CRC32), y)
MODULE_DEPS += lib/sd_crc32
endif

ifeq ($(CONFIG_SD_X509), y)
MODULE_DEPS += lib/sd_x509
endif

ifeq ($(CONFIG_PART_LOAD_UTIL), y)
MODULE_DEPS += lib/partition_load
endif

ifeq ($(CONFIG_VERIFIED_BOOT), y)
MODULE_DEPS += lib/verified_boot
endif

ifeq ($(CONFIG_LIBAVB), y)
MODULE_DEPS += lib/libavb
endif

ifeq ($(CONFIG_DEBUG), y)
MODULE_DEPS += lib/debug
endif

ifeq ($(CONFIG_DEBUG_COMMANDS), y)
MODULE_DEPS += lib/debugcommands
endif

ifeq ($(CONFIG_DISK), y)
MODULE_DEPS += lib/disk
endif

ifeq ($(CONFIG_FEE), y)
MODULE_DEPS += lib/fee
endif

ifeq ($(CONFIG_LIBC), y)
MODULE_DEPS += lib/libc
endif

ifeq ($(CONFIG_PARTITION), y)
MODULE_DEPS += lib/partition
endif

ifeq ($(CONFIG_RES_LOADER), y)
MODULE_DEPS += lib/res_loader
endif

ifeq ($(CONFIG_RING_BUFFER), y)
MODULE_DEPS += lib/ring_buffer
endif

ifeq ($(CONFIG_SD_BOOT_IMG), y)
MODULE_DEPS += lib/sd_boot_img
endif

ifeq ($(CONFIG_SD_BOOT_PIN), y)
MODULE_DEPS += lib/sd_boot_pin
endif

ifeq ($(CONFIG_SLIMFS), y)
MODULE_DEPS += lib/slimfs
endif

ifeq ($(CONFIG_WORKQUEUE), y)
MODULE_DEPS += lib/workqueue
endif

ifeq ($(CONFIG_RES_LOADER), y)
MODULE_DEPS += lib/res_loader
endif

ifeq ($(CONFIG_ELF_LOADER), y)
MODULE_DEPS += lib/elf
endif
