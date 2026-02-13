COMPONENT_SRCDIRS := nanopb src
COMPONENT_INCDIRS := nanopb src/include

COMPONENT_SUBMODULES += nanopb

NANOPB_GENERATE := $(PYTHON) $(COMPONENT_PATH)/nanopb/generator/nanopb_generator.py
