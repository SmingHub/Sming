CFLAGS += -Wno-format-nonliteral -Wno-override-init -Wno-pedantic
NO_AUTO_SRC := 1
SRC += bpf.c
SRC += call.c
SRC += store.c
SRC += verify.c

BPF_USE_JUMPTABLE ?= 1

ifeq ($(BPF_USE_JUMPTABLE), 1)
  SRC += jumptable.c
else
  SRC += instruction.c
endif

include $(RIOTBASE)/Makefile.base
