# use linker garbage collection, if requested
#ifeq ($(CONFIG_WITH_LINKER_GC), y)
GLOBAL_COMPILEFLAGS += -ffunction-sections -fdata-sections
GLOBAL_LDFLAGS += --gc-sections
#endif

ifneq (,$(EXTRA_BUILDRULES))
-include $(EXTRA_BUILDRULES)
endif

# generate git commit id
BUILD_COMMIT_ID := $(shell git rev-parse --short HEAD)
# dirty 检测：工作区有未提交修改 => 版本串加 -dirty（可区分同 commit 的本地构建）
BUILD_DIRTY_MARK := $(shell git status --porcelain 2>/dev/null | head -c 1)
ifneq ($(BUILD_DIRTY_MARK),)
BUILD_VERSION_TAG := $(BUILD_COMMIT_ID)-dirty
else
BUILD_VERSION_TAG := $(BUILD_COMMIT_ID)
endif
#
# generate time info
BUILD_TIMESTAMP := $(shell date +"%F %T")

# debug or release version
ifeq ($(CONFIG_RELEASE_VER), y)
BUILD_SELECT_VERSION := RELEASE
else ifeq ($(CONFIG_DEBUG_VER), y)
BUILD_SELECT_VERSION := DEBUG
else
BUILD_SELECT_VERSION :=
endif

$(EXTRA_LINKER_SCRIPTS):

$(OUTBIN): $(OUTELF)
	@echo generating image: $@
	$(NOECHO)$(SIZE) $<
	$(NOECHO)$(OBJCOPY) -O binary $< $@

ifeq ($(CONFIG_AUTO_SIGN_BOARD_IMG), y)
	$(warning "start gen sign bin $(OUTBIN), membase is $(CONFIG_MEM_BASE)")
	$(NOECHO)tools/sdtools/gen_sign_bin $(OUTBIN) $(CONFIG_MEM_BASE)
endif

$(OUTELF).hex: $(OUTELF)
	@echo generating hex file: $@
	$(NOECHO)$(OBJCOPY) -O ihex $< $@

$(warning "the link script is $(LINKER_SCRIPT)")

$(OUTELF): $(ALLMODULE_OBJS) $(EXTRA_OBJS) $(LINKER_SCRIPT) $(EXTRA_LINKER_SCRIPTS)
	@echo linking $@
	@echo '#include "ssdk_version.h"' > $(BUILDDIR)/build_msg.c
	@echo 'const char build_msg[] = "SSDK Built: " SSDK_VERSION " - ${BUILD_TIMESTAMP} - ${BUILD_SELECT_VERSION}";' >> $(BUILDDIR)/build_msg.c
	$(CC) $(GLOBAL_OPTFLAGS) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(GLOBAL_CFLAGS) \
		$(ARCH_CFLAGS) $(THUMBCFLAGS) $(GLOBAL_INCLUDES) -c $(BUILDDIR)/build_msg.c -o $(BUILDDIR)/build_msg.o
	$(NOECHO)$(SIZE) -t --common $(sort $(ALLMODULE_OBJS)) $(EXTRA_OBJS)
	$(NOECHO)$(LD) -dT $(LINKER_SCRIPT) $(addprefix -T,$(EXTRA_LINKER_SCRIPTS)) \
		--start-group $(ALLMODULE_OBJS) $(EXTRA_OBJS) $(BUILDDIR)/build_msg.o --end-group $(LIBGCC) $(GLOBAL_LDFLAGS) -Map=$(OUTELF).map -o $@

$(OUTELF).sym: $(OUTELF)
	@echo generating symbols: $@
	$(NOECHO)$(OBJDUMP) -t $< | $(CPPFILT) > $@

$(OUTELF).sym.sorted: $(OUTELF)
	@echo generating sorted symbols: $@
	$(NOECHO)$(OBJDUMP) -t $< | $(CPPFILT) | sort > $@

$(OUTELF).lst: $(OUTELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -d $< | $(CPPFILT) > $@

$(OUTELF).debug.lst: $(OUTELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -S $< | $(CPPFILT) > $@

$(OUTELF).dump: $(OUTELF)
	@echo generating objdump: $@
	$(NOECHO)$(OBJDUMP) -x $< > $@

$(OUTELF).size: $(OUTELF)
	@echo generating size map: $@
	$(NOECHO)$(NM) -S --size-sort $< > $@

# print some information about the build
$(BUILDDIR)/srcfiles.txt: $(OUTELF)
	@echo generating $@
	$(NOECHO)echo $(sort $(ALLSRCS)) | tr ' ' '\n' > $@

$(BUILDDIR)/include_paths.txt: $(OUTELF)
	@echo generating $@
	$(NOECHO)echo $(subst -I,,$(sort $(GLOBAL_INCLUDES))) | tr ' ' '\n' > $@
