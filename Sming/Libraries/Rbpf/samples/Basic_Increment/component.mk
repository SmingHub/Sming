COMPONENT_DEPENDS := Rbpf

COMPONENT_INCDIRS += blobs

blobs/increment.bin.h: blobs

CUSTOM_TARGETS += blobs/increment.bin.h
