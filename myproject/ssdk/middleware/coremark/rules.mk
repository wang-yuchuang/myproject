LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

# Number of iterations
ITERATIONS ?= 30000

# SSDK porting
PORT_DIR := ssdk

MODULE_INCLUDES += $(LOCAL_DIR)/$(PORT_DIR)
MODULE_SRCS += $(LOCAL_DIR)/$(PORT_DIR)/core_portme.c

# MODULE_OPTFLAGS = -O3

# CoreMark core
FLAGS_STR = "$(GLOBAL_OPTFLAGS)"
MODULE_COMPILEFLAGS += \
	-DFLAGS_STR=\"$(FLAGS_STR)\" \
	-DITERATIONS=$(ITERATIONS) \
	-DPERFORMANCE_RUN=1

MODULE_SRCS += \
	$(LOCAL_DIR)/core_list_join.c \
	$(LOCAL_DIR)/core_main.c \
	$(LOCAL_DIR)/core_matrix.c \
	$(LOCAL_DIR)/core_state.c \
	$(LOCAL_DIR)/core_util.c

MODULE_INCLUDES += $(LOCAL_DIR)

include $(MKROOT)/module.mk
