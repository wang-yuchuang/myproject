LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
    ${LOCAL_DIR}/src \
    ${LOCAL_DIR}/inc \

MODULE_SRCS += \
    $(shell find -L $(LOCAL_DIR)/inc -name *.cpp) \
    $(shell find -L $(LOCAL_DIR)/src -name *.cpp) \
    $(shell find -L $(LOCAL_DIR)/src -name *.S) \
    $(shell find -L $(LOCAL_DIR)/src -name *.bin) \

GLOBAL_DEFINES += ENABLE_QT_APP
GLOBAL_DEFINES += QUL_STD_STRING_SUPPORT

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-use-cxa-atexit -nostdlib

include $(MKROOT)/module.mk
