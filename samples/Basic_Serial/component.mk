SPIFF_SIZE      ?= 65536

CUSTOM_TARGETS	:= files/Readme.md

# Large text file for demo purposes
files/Readme.md: $(SMING_HOME)/../Readme.md
	$(Q) mkdir -p $(@D)
	$(Q) cp $< $@
