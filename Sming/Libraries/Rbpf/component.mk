COMPONENT_INCDIRS := include/

COMPONENT_CFLAGS := -Wno-format-nonliteral -Wno-override-init -Wno-pedantic
COMPONENT_SRCFILES := \
			src/bpf/bpf.c \
			src/bpf/call.c \
			src/bpf/store.c \
			src/bpf/verify.c \
			\
			src/memarray/memarray.c \
			\
			src/btree/btree.c

CONFIG_VARS := BPF_USE_JUMPTABLE
BPF_USE_JUMPTABLE := 1

ifeq ($(BPF_USE_JUMPTABLE), 1)
  COMPONENT_SRCFILES += src/bpf/jumptable.c
else
  COMPONENT_SRCFILES += src/bpf/instruction.c
endif

# The folder where the container application source code is stored.
CONTAINER_FOLDER := container

##@Rbpf containers

BLOB_FOLDER := blobs

.PHONY: blobs
blobs: $(CONTAINER_FOLDER) ##Compiles container blobs 
	$(Q) mkdir -p $@ 
	$(Q) (cd $<; $(MAKE) --silent -f $(COMPONENT_PATH)/rbpf.inc.mk BLOB_FOLDER=$(CURDIR)/$@) 
	
.PHONY: blobs-clean
blobs-clean: $(CONTAINER_FOLDER) ##Cleans container blobs 
	$(Q) rm -rf $(BLOB_FOLDER)
	$(Q) (cd $<; $(MAKE) --silent -f $(COMPONENT_PATH)/rbpf.inc.mk clean)
	
CUSTOM_TARGETS += blobs