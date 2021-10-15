COMPONENT_DOCFILES := \
	uf2.md \
	uf2conv.md

# Path to python utility for manipulating files and uploading to target
DEBUG_VARS += UF2CONV_PY
UF2CONV_PY := $(COMPONENT_PATH)/uf2conv.py


# Invoke uf2conf utility
# $1 -> Parameters
ifdef WSL_ROOT
Uf2Conv = powershell.exe -Command "$(PYTHON) $(UF2CONV_PY) $(if $V,--verbose) $1"
else
Uf2Conv = $(PYTHON) $(UF2CONV_PY) $(if $V,--verbose) $1
endif


# Read flash manufacturer ID and determine actual size
define ReadFlashID
	$(info Reading Flash ID)
	$(call Uf2Conv,--list --verbose)
endef

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(if $1,\
		$(info WriteFlash $1) \
		$(call Uf2Conv,--upload $1 --output $(OUT_BASE)/flash.uf2)
	)
endef

# Verify flash against file contents
# $1 -> List of `Offset=File` chunks
define VerifyFlash
	$(if $1,\
		$(info VerifyFlash $1)
		$(info ** NOT IMPLEMENTED **)
	)
endef

# Read flash memory into file
# $1 -> `Offset,Size` chunk
# $2 -> Output filename
define ReadFlash
	$(info ReadFlash $1,$2)
	$(info ** NOT IMPLEMENTED **)
endef

# Erase a region of Flash
# $1 -> Offset,Size
define EraseFlashRegion
	$(info EraseFlashRegion $1)
	$(info ** NOT IMPLEMENTED **)
endef

# Erase flash memory contents
define EraseFlash
	$(info ** NOT IMPLEMENTED **)
endef
