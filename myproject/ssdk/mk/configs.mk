CONFIG_ROOT_DIR := $(COREDIR)/configs
CONFIG_PRJ_DIR := $(COREDIR)/configs/$(PROJECT)
CONFIG_OUT_DIR := $(BUILDDIR)/configs

CONFIG_PRJ_FILES := $(notdir $(wildcard $(CONFIG_PRJ_DIR)/*.c $(CONFIG_PRJ_DIR)/*.h))
CONFIG_DEF_FILES = $(notdir $(wildcard $(CONFIG_ROOT_DIR)/*.c $(CONFIG_ROOT_DIR)/*.h))
CONFIG_DEF_FILES := $(filter-out $(CONFIG_PRJ_FILES),$(CONFIG_DEF_FILES))

CONFIG_RPJ_OUT_FILES := $(addprefix $(CONFIG_OUT_DIR)/,$(CONFIG_PRJ_FILES))
CONFIG_DEF_OUT_FILES := $(addprefix $(CONFIG_OUT_DIR)/,$(CONFIG_DEF_FILES))
CONFIG_OUT_FILES := $(CONFIG_RPJ_OUT_FILES) $(CONFIG_DEF_OUT_FILES)

CONFIG_HEADER_FILES = $(filter %.h,$(CONFIG_OUT_FILES))
CONFIG_SRC_FILES = $(filter %.c,$(CONFIG_OUT_FILES))
export CONFIG_HEADER_FILES

# copy root config file to build dir
$(filter %.c,$(CONFIG_RPJ_OUT_FILES)): $(CONFIG_OUT_DIR)/%.c: $(CONFIG_PRJ_DIR)/%.c
	@$(call MAKEDEVICECONFIG,$^,$@)

$(filter %.h,$(CONFIG_RPJ_OUT_FILES)): $(CONFIG_OUT_DIR)/%.h: $(CONFIG_PRJ_DIR)/%.h
	@$(call MAKEDEVICECONFIG,$^,$@)

$(filter %.c,$(CONFIG_DEF_OUT_FILES)): $(CONFIG_OUT_DIR)/%.c: $(CONFIG_ROOT_DIR)/%.c
	@$(call MAKEDEVICECONFIG,$^,$@)

$(filter %.h,$(CONFIG_DEF_OUT_FILES)): $(CONFIG_OUT_DIR)/%.h: $(CONFIG_ROOT_DIR)/%.h
	@$(call MAKEDEVICECONFIG,$^,$@)

# add config files to module build
GENERATED += $(CONFIG_OUT_FILES)

MODULE := boards/configs

MODULE_SRCS += $(CONFIG_SRC_FILES)
MODULE_SRCDEPS += $(CONFIG_HEADER_FILES)

GLOBAL_INCLUDES += $(CONFIG_OUT_DIR)

make_configs: $(CONFIG_OUT_FILES)

.PHONY: make_configs

include $(MKROOT)/module.mk
