COMPONENT_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

RBPF_SOURCES ?= $(wildcard $(CURDIR)/*.c)
RBPF_GENRBF := $(COMPONENT_PATH)/tools/gen_rbf.py

RBPF_BINS = $(RBPF_SOURCES:.c=.bin)
RBPF_OBJS = $(RBPF_SOURCES:.c=.o)

BLOB_FOLDER ?= ..

LLC ?= llc
CLANG ?= clang
XXD ?= xxd
INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`
EXTRA_CFLAGS ?= -Os -emit-llvm

RBPFINCLUDE =  -I$(RIOTBASE)/drivers/include \
	       -I$(RIOTBASE)/core/include \
	       -I$(RIOTBASE)/sys/include \
	       #

all: blobs

.PHONY: clean

clean:
	rm -f $(RBPF_OBJS)

INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`

$(RBPF_OBJS):  %.o:%.c
	$(CLANG) $(INC_FLAGS) \
	        $(RBPFINCLUDE) \
	        -Wno-unused-value -Wno-pointer-sign -g3\
	        -Wno-compare-distinct-pointer-types \
	        -Wno-gnu-variable-sized-type-not-at-end \
	        -Wno-address-of-packed-member -Wno-tautological-compare \
	        -Wno-unknown-warning-option \
	        $(EXTRA_CFLAGS) -c $< -o -| $(LLC) -march=bpf -mcpu=v2 -filetype=obj -o $@

$(RBPF_BINS): %.bin:%.o
	$(RBPF_GENRBF) generate $< $@
	
.PHONY: blobs
blobs: $(RBPF_BINS)
	$(XXD) -i $(notdir $<) | sed 's/^unsigned/const unsigned/g'> $(BLOB_FOLDER)/$(notdir $<).h
	
.PHONY: dump
dump: $(RBPF_BINS)
	$(RBPF_GENRBF) dump $< 
