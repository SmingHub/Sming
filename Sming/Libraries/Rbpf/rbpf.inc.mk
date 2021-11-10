include $(SMING_HOME)/util.mk

ifeq (,$(RBPF_BLOBDIR))
$(error RBPF_BLOBDIR undefined)
endif
ifeq (,$(RBPF_INCDIR))
$(error RBPF_INCDIR undefined)
endif

# List of relative paths to source files
RBPF_SOURCES	:= $(patsubst $(CURDIR)/%,%,$(call ListAllFiles,$(CURDIR),*.c))
# BPF object file
RBPF_OBJS		:= $(RBPF_SOURCES:.c=.o)
# Object code blob
RBPF_BINS		:= $(RBPF_SOURCES:.c=.bin)
# Header file for each souce with IMPORT_FSTR statement
RBPF_INCFILES	:= $(addprefix $(RBPF_INCDIR)/,$(RBPF_SOURCES:.c=.h))

LLC ?= llc
CLANG ?= clang
XXD ?= xxd
INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`
EXTRA_CFLAGS ?= -Os -emit-llvm

RBPF_INCLUDE :=
# RBPF_INCLUDE = \
# 	-I$(RIOTBASE)/drivers/include \
# 	-I$(RIOTBASE)/core/include \
# 	-I$(RIOTBASE)/sys/include

all: blobs

.PHONY: clean

clean:
	$(Q) rm -rf $(RBPF_BLOBDIR) $(RBPF_INCDIR)

INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`

# $1 -> Source file
define BpfIncFile
$(RBPF_INCDIR)/rbpf/container/$(1:.c=.h)
endef

# Generated build targets
# $1 -> Source file
# $2 -> Object file
# $3 -> Header file
define GenerateTarget
O_FILE := $(RBPF_BLOBDIR)/$(1:.c=.o)
BIN_FILE := $$(O_FILE:.o=.bin)
FSTR_SYMNAME := $(subst /,_,$(1:.c=))
$$(O_FILE): $1
	$(Q) mkdir -p $$(@D)
	$(Q) $$(CLANG) \
		$$(INC_FLAGS) \
		$$(RBPF_INCLUDE) \
		-Wno-unused-value -Wno-pointer-sign -g3\
		-Wno-compare-distinct-pointer-types \
		-Wno-gnu-variable-sized-type-not-at-end \
		-Wno-address-of-packed-member -Wno-tautological-compare \
		-Wno-unknown-warning-option \
		$$(EXTRA_CFLAGS) -c $$< -o -| $$(LLC) -march=bpf -mcpu=v2 -filetype=obj -o $$@
$$(BIN_FILE): $$(O_FILE)
	$$(RBPF_GENRBF) generate $$< $$@
$(call BpfIncFile,$1): $$(BIN_FILE)
	@mkdir -p $$(@D)
	@echo "#pragma once" > $$@
	@echo "#include <FlashString/Array.hpp>" >> $$@
	@echo "" >> $$@
	@echo "namespace rBPF {" >> $$@
	@echo "namespace Container {" >> $$@
	@printf "IMPORT_FSTR_ARRAY($$(FSTR_SYMNAME), uint8_t, \"$$<\")\n" >> $$@
	@echo "} // namespace Container" >> $$@
	@echo "} // namespace rBPF" >> $$@
endef
$(foreach f,$(RBPF_SOURCES),$(eval $(call GenerateTarget,$f,)))

.PHONY: blobs
blobs: $(foreach f,$(RBPF_SOURCES),$(call BpfIncFile,$f))

.PHONY: dump
dump: blobs
	$(RBPF_GENRBF) dump $< 
