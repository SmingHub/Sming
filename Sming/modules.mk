#####
#
# Module support
#
# Both Sming and Application makefiles include this once all modules, libs, etc. have been defined
#
#####

# List of directories containing object files
BUILD_DIR :=

# List of object files
OBJ :=

# Intermediate linker target
APP_AR	:= $(BUILD_BASE)/$(TARGET).a

INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))

# $1 -> directory containing source files
# $2 -> output build directory
define GenerateCompileTargets
$2/%.o: $1/%.s
	$(vecho) "AS $$<"
	$(Q) $(AS) $(INCDIR) $(CFLAGS) -c $$< -o $$@
$2/%.o: $1/%.S
	$(vecho) "AS $$<"
	$(Q) $(AS) $(INCDIR) $(CFLAGS) -c $$< -o $$@
$2/%.o: $1/%.c $2/%.c.d
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(CFLAGS) -std=c11 -c $$< -o $$@
$2/%.o: $1/%.cpp $2/%.cpp.d
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(INCDIR) $(CXXFLAGS) -c $$< -o $$@
$2/%.c.d: $1/%.c
	$(Q) $(CC) $(INCDIR) $(CFLAGS) -std=c11 -MM -MT $2/$$*.o $$< -o $$@
$2/%.cpp.d: $1/%.cpp
	$(Q) $(CXX) $(INCDIR) $(CXXFLAGS) -MM -MT $2/$$*.o $$< -o $$@

.PRECIOUS: $2/%.c.d $2/%.cpp.d
endef


# Append target object file paths to OBJ
# $1 -> directory containing source files
# $2 -> output build directory
# $3 -> file extension
define ScanObjectFiles
	OBJ += $(addprefix $2/,$(patsubst $(SMING_HOME)/%,%,$(patsubst %$3,%.o,$(wildcard $1/*$3))))
endef

# Generate all the compile target patterns and update object list for all source files in a module directory
# $1 -> directory containing source files
# $2 -> output build directory
define ScanModule
	$(if $(V), $(info Scan $1))
	BUILD_DIR += $2/$(patsubst $(SMING_HOME)/%,%,$1)
	$(eval $(call ScanObjectFiles,$1,$2,.s))
	$(eval $(call ScanObjectFiles,$1,$2,.S))
	$(eval $(call ScanObjectFiles,$1,$2,.c))
	$(eval $(call ScanObjectFiles,$1,$2,.cpp))
	$(eval $(call GenerateCompileTargets,$1,$(BUILD_BASE)/$(patsubst $(SMING_HOME)/%,%,$1)))
endef

# $1 -> directory containing source files
define ScanModules
	$(foreach sdir,$1,$(eval $(call ScanModule,$(sdir),$(BUILD_BASE))))
endef

# Generate all the compile target patterns and object references
$(call ScanModules,$(MODULES))
