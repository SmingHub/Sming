COMPONENT_SUBMODULES := bearssl

COMPONENT_SRCDIRS := \
	. \
	aead \
	codec \
	ec \
	hash \
	int \
	kdf \
	mac \
	rand \
	rsa \
	ssl \
	symcipher \
	x509

COMPONENT_SRCDIRS := $(addprefix bearssl/src/,$(COMPONENT_SRCDIRS))
COMPONENT_INCDIRS := bearssl/inc
EXTRA_INCDIR := bearssl/src

COMPONENT_CFLAGS := \
	-Wno-undef \
	-O2
