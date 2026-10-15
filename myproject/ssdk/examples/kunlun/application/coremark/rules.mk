LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/core_main.c \
	$(LOCAL_DIR)/src/core_list_join.c \
	$(LOCAL_DIR)/src/core_matrix.c \
	$(LOCAL_DIR)/src/core_state.c \
	$(LOCAL_DIR)/src/core_util.c \
	$(LOCAL_DIR)/src/core_portme.c \
	$(LOCAL_DIR)/src/coremark_app.c \

# -O2 is required for a meaningful CoreMark score (reported in FLAGS_STR).
# -Dmain=coremark_main renames the official main() so the benchmark can be
# invoked from the "coremark" CLI command instead of the platform entry.
# -fno-builtin prevents the compiler from turning benchmark loops into
# library calls (same policy as the other example apps).
MODULE_OPTFLAGS := -O2
MODULE_COMPILEFLAGS += -fno-builtin -Dmain=coremark_main \
	-Dcrc16=coremark_crc16 \
	-Wno-format -Wno-unused-variable -Wno-unused-but-set-variable \
	-Wno-sign-compare

include $(MKROOT)/module.mk
