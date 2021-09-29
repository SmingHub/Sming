###
#
# SMING Application Makefile for RP2040 architecture
#
###

# linker flags used to generate the main object file
LDFLAGS	+= \
	-Wl,--build-id=none \
	--specs=nosys.specs \
	-mcpu=cortex-m0plus \
	-mthumb

TARGET_DIS = $(TARGET_OUT:.out=.dis)
TARGET_SYM = $(TARGET_OUT:.out=.sym)

.PHONY: application
application: $(TARGET_BIN)

$(TARGET_OUT): $(COMPONENTS_AR)
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@ $(BOOTLOADER)
	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO)
	$(Q) cat $(FW_MEMINFO)

$(TARGET_BIN): $(TARGET_OUT)
	@# Disassembly
	$(Q) $(OBJDUMP) -h -C $< > $(TARGET_DIS)
	$(Q) $(OBJDUMP) -d -S -C $< >> $(TARGET_DIS)
	@# Symbols
	@printf "Symbols by name\n" > $(TARGET_SYM)
	$(Q) $(NM) -S -l -C $< >> $(TARGET_SYM)
	@printf "\n\nSymbols by address\n" >> $(TARGET_SYM)
	$(Q) $(NM) -S -l -n -C $< >> $(TARGET_SYM)
	@printf "\n\nSymbols by size\n">> $(TARGET_SYM)
	$(Q) $(NM) -S -l --size-sort -C $< >> $(TARGET_SYM)
	@# Binary output
	$(Q) $(OBJCOPY) -O binary $< $@
