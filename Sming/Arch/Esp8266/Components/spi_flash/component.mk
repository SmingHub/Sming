
# => SMING

MODULES			+= $(ARCH_COMPONENTS)/spi_flash
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/spi_flash/include


# => APP

EXTRA_INCDIR += $(ARCH_COMPONENTS)/spi_flash/include