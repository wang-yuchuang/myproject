LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_CLI), y)
MODULE_DEPS += middleware/CLI
endif

ifeq ($(CONFIG_MEM_RW), y)
MODULE_DEPS += middleware/mem_rw
endif

ifeq ($(CONFIG_DHRYSTONE), y)
MODULE_DEPS += middleware/dhrystone
endif

ifeq ($(CONFIG_COREMARK), y)
MODULE_DEPS += middleware/coremark
endif

ifeq ($(CONFIG_FLASH_LOADER), y)
MODULE_DEPS += middleware/flashloader
endif

ifeq ($(CONFIG_RPMSG), y)
MODULE_DEPS += middleware/rpmsg
endif

ifeq ($(CONFIG_AUDIO_MANAGER), y)
MODULE_DEPS += middleware/audiomanager
MODULE_DEPS += middleware/audio_service
endif

ifeq ($(CONFIG_AUDIO_HUB), y)
MODULE_DEPS += middleware/audio_hub
MODULE_DEPS += middleware/audiomanager
MODULE_DEPS += middleware/audio_service
endif

ifeq ($(CONFIG_RPMSG_SERVICE), y)
MODULE_DEPS += middleware/rpmsg_service
endif

ifeq ($(CONFIG_SDPE_RPC), y)
MODULE_DEPS += middleware/sdpe_rpc
endif

ifeq ($(CONFIG_MEM_IMAGE), y)
MODULE_DEPS += middleware/mem_image
endif

ifeq ($(CONFIG_REBOOT), y)
MODULE_DEPS += middleware/reboot
endif

ifeq ($(CONFIG_FATFS), y)
MODULE_DEPS += middleware/fatfs
endif

ifeq ($(CONFIG_UPDATE_MONITOR), y)
MODULE_DEPS += middleware/update_monitor
GLOBAL_INCLUDES += middleware/update_monitor
endif

ifeq ($(CONFIG_GUI_LVGL), y)
MODULE_DEPS += middleware/littlevgl
endif

ifeq ($(CONFIG_DLOADER), y)
MODULE_DEPS += middleware/dloader
endif

ifeq ($(CONFIG_CU), y)
MODULE_DEPS += middleware/cu
endif

ifeq ($(CONFIG_TJPGDEC), y)
MODULE_DEPS += middleware/tjpgdec
endif

ifeq ($(CONFIG_MJPEG), y)
MODULE_DEPS += middleware/mjpeg
endif

ifeq ($(CONFIG_MP3_DEC), y)
MODULE_DEPS += middleware/minimp3
endif

ifeq ($(CONFIG_FLOPS), y)
MODULE_DEPS += middleware/flops
endif

ifeq ($(CONFIG_DECODER_LODEPNG), y)
MODULE_DEPS += middleware/lodepng
endif

ifeq ($(CONFIG_GPU_HAL), y)
MODULE_DEPS += middleware/gpu_hal
endif

ifeq ($(CONFIG_FUSA_MONITOR), y)
MODULE_DEPS += middleware/fusa_monitor
endif

ifeq ($(CONFIG_CPU_FREQ), y)
MODULE_DEPS += middleware/cpufreq
endif

ifeq ($(CONFIG_VLIN), y)
MODULE_DEPS += middleware/vlin
endif

ifeq ($(CONFIG_VCAN), y)
MODULE_DEPS += middleware/vcan
endif

ifeq ($(CONFIG_VI2C), y)
MODULE_DEPS += middleware/vi2c
endif

ifeq ($(CONFIG_USB), y)
MODULE_DEPS += middleware/usb
endif

ifeq ($(CONFIG_BOARDINFO), y)
MODULE_DEPS += middleware/boardinfo
endif

ifeq ($(CONFIG_CORE_DEBUG), y)
MODULE_DEPS += middleware/coredbg
endif

ifeq ($(CONFIG_IRAM_NONE_ECC), y)
MODULE_DEPS += middleware/iram
endif

ifeq ($(CONFIG_APP_REBOOT_SS), y)
MODULE_DEPS += middleware/app_reboot_ss
endif

ifeq ($(CONFIG_BACKLIGHT_SERVICE), y)
MODULE_DEPS += middleware/backlight
endif

ifeq ($(CONFIG_KUNLUN_CAMERA_SERVICE), y)
MODULE_DEPS += middleware/camera
endif

ifeq ($(CONFIG_CAN_PROXY), y)
MODULE_DEPS += middleware/can_proxy
endif

ifeq ($(CONFIG_CLUSTER_BRIDGE), y)
MODULE_DEPS += middleware/cluster_bridge
endif

ifeq ($(CONFIG_DISP_CLIENT), y)
MODULE_DEPS += middleware/disp_client
endif

ifeq ($(CONFIG_DISPLAY_SERVICE), y)
MODULE_DEPS += middleware/display
endif

ifeq ($(CONFIG_AVMCAST_SERVICE), y)
MODULE_DEPS += middleware/avmcast
endif

ifeq ($(CONFIG_INPUT_SERVICE), y)
MODULE_DEPS += middleware/input
endif

ifeq ($(CONFIG_LWIP), y)
MODULE_DEPS += middleware/lwip
endif

ifeq ($(CONFIG_NFS_CLIENT), y)
MODULE_DEPS += middleware/nfs_client
endif

ifeq ($(CONFIG_PROPERTY), y)
MODULE_DEPS += middleware/sd_property
endif

ifeq ($(CONFIG_ERRCODE), y)
MODULE_DEPS += middleware/sd_errcode
else
MODULE_DEPS += middleware/sd_errcode/sd_errcode_weak
endif

ifeq ($(CONFIG_SYS_DIAGNOSIS), y)
MODULE_DEPS += middleware/sys_diagnosis
endif

ifeq ($(CONFIG_QT_FOR_MCU), y)
MODULE_DEPS += middleware/qt_for_mcu
endif

ifeq ($(CONFIG_SDRV_VIRT_CONSOLE), y)
MODULE_DEPS += middleware/sdshell
endif

ifeq ($(CONFIG_FuncSafetyPkg), y)
MODULE_DEPS += middleware/FuncSafetyPkg
endif

ifeq ($(CONFIG_SDRV_THERMAL_SERVICE), y)
MODULE_DEPS += middleware/thermal_service
endif

ifeq ($(CONFIG_VPU_SRV), y)
MODULE_DEPS += middleware/vpu_hal/vpu_instance
MODULE_DEPS += middleware/vpu_service
endif

ifeq ($(CONFIG_VPU_CLIENT), y)
MODULE_DEPS += middleware/vpu_hal/vpu_client
endif

ifeq ($(CONFIG_ENABLE_EXCEPTION_LOG_FLAG), y)
MODULE_DEPS += middleware/logflag
endif

ifeq ($(CONFIG_GRAPHICS), y)
MODULE_DEPS += middleware/sd_graphics
endif

ifeq ($(CONFIG_VIRTIO), y)
MODULE_DEPS += middleware/virtio
endif

ifeq ($(CONFIG_TIME_SYNC), y)
MODULE_DEPS += middleware/time_sync
endif

ifeq ($(CONFIG_STR_CORE), y)
MODULE_DEPS += middleware/str
endif

ifeq ($(CONFIG_DW_UMCTL2), y)
MODULE_DEPS += middleware/dw_umctl2
endif

ifeq ($(CONFIG_MODULE_HELPER), y)
MODULE_DEPS += middleware/module_helper
endif

ifeq ($(CONFIG_SDRV_SHM), y)
MODULE_DEPS += middleware/share_memory
endif

ifeq ($(CONFIG_MARK_SUCCESS), y)
MODULE_DEPS += middleware/mark_success
endif

ifeq ($(CONFIG_MEM_TESTER), y)
MODULE_DEPS += middleware/memtester
endif

ifeq ($(CONFIG_ICCMSG), y)
MODULE_DEPS += middleware/iccmsg
endif


