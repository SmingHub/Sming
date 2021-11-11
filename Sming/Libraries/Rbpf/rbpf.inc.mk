include $(SMING_HOME)/util.mk

ifeq (,$(RBPF_BLOBDIR))
$(error RBPF_BLOBDIR undefined)
endif
ifeq (,$(RBPF_INCDIR))
$(error RBPF_INCDIR undefined)
endif

# Obtain blob file path
# $1 -> source file(s)
define BlobFile
$(addprefix $(RBPF_BLOBDIR)/,$(patsubst %,%.bin,$(basename $1)))
endef

# List of relative paths to source files
RBPF_SOURCES	:= $(patsubst $(CURDIR)/%,%,$(call ListAllFiles,$(CURDIR),*.c *.cpp))
# Header file for all defined containers
RBPF_INCFILE	:= $(RBPF_INCDIR)/rbpf/containers.h

LLC ?= llc
CLANG ?= clang
XXD ?= xxd
INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`
EXTRA_CFLAGS ?= -Os -emit-llvm

all: blobs

.PHONY: clean

clean:
	$(Q) rm -rf $(RBPF_BLOBDIR) $(RBPF_INCDIR)

INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`

# Generated build targets
# $1 -> Source file
# $2 -> Blob file
define GenerateTarget
$(2:.bin=.o): $1
	$(Q) mkdir -p $$(@D)
	$(Q) $$(CLANG) \
		$$(INC_FLAGS) \
		-Wno-unused-value -Wno-pointer-sign -g3\
		-Wno-compare-distinct-pointer-types \
		-Wno-gnu-variable-sized-type-not-at-end \
		-Wno-address-of-packed-member -Wno-tautological-compare \
		-Wno-unknown-warning-option \
		$$(EXTRA_CFLAGS) -c $$< -o -| $$(LLC) -march=bpf -mcpu=v2 -filetype=obj -o $$@
$2: $(2:.bin=.o)
	$$(RBPF_GENRBF) generate $$< $$@
endef
$(foreach f,$(RBPF_SOURCES),$(eval $(call GenerateTarget,$f,$(call BlobFile,$f))))


# Get name to use for blob symbol
# $1 -> source file
define GetSymbolName
$(subst /,_,$(basename $1))
endef

# Generate code for header file
# $1 -> source file
define GenerateHeader
@printf "IMPORT_FSTR_ARRAY($(call GetSymbolName,$1), uint8_t, \"$(call BlobFile,$1)\")\n" >> $@

endef

$(RBPF_INCFILE): $(call BlobFile,$(RBPF_SOURCES))
	@mkdir -p $(@D)
	@echo "#pragma once" > $@
	@echo "#include <FlashString/Array.hpp>" >> $@
	@echo "" >> $@
	@echo "namespace rBPF {" >> $@
	@echo "namespace Container {" >> $@
	$(foreach f,$(RBPF_SOURCES),$(call GenerateHeader,$f))
	@echo "} // namespace Container" >> $@
	@echo "} // namespace rBPF" >> $@


.PHONY: blobs
blobs: $(RBPF_INCFILE)

.PHONY: dump
dump: blobs
	$(RBPF_GENRBF) dump $< 
