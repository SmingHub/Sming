COMPONENT_LIBNAME :=

COMPONENT_DOCFILES := \
	uf2.md \
	uf2conv.md

# Path to python utility for manipulating files and uploading to target
DEBUG_VARS += UF2CONV_PY
UF2CONV_PY := $(COMPONENT_PATH)/uf2conv.py

ifeq ($(SMING_SOC),rp2350)
UF2_FAMILY := "rp2xxx_absolute"
else
UF2_FAMILY := "rp2040"
endif

# Invoke uf2conv utility
# $1 -> Parameters
ifdef WSL_ROOT
Uf2Conv = powershell.exe -Command "$(PYTHON) $(UF2CONV_PY) $(if $V,--verbose) $1"
else
Uf2Conv = $(PYTHON) $(UF2CONV_PY) $(if $V,--verbose) $1
endif

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(if $1,\
		$(info WriteFlash $1) \
		$(call Uf2Conv,--family $(UF2_FAMILY) --upload $1 --output $(OUT_BASE)/flash.uf2)
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
