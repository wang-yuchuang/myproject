LOCAL_DIR := $(GET_LOCAL_DIR)

# add deps for iar compiler
ifeq ($(SSDK_TOOLCHAIN),iar)
include $(LOCAL_DIR)/iar/rules.mk
else 
# add deps for gcc and rapidstudio compiler
include $(LOCAL_DIR)/newlib/rules.mk
endif
