COMPONENT_INCDIRS		:= include
COMPONENT_SRCDIRS		:= src

COMPONENT_DOXYGEN_INPUT	:= include/sys

EXTRA_LIBS				+= m c gcc

ifndef MAKE_CLEAN

# build customised libstdc++
# https://github.com/esp8266/Arduino/blob/master/tools/sdk/lib/README.md
LIBSTDCPP_SRC = $(call FixPath,$(shell $(CC) -print-file-name=libstdc++.a))
LIBSTDCPP_DST = $(USER_LIBDIR)/libstdc++.a

$(COMPONENT_RULE)$(LIBSTDCPP_DST): $(LIBSTDCPP_SRC)
	$(info Prepare libstdc++)
	$(Q) cp $< $@
	$(Q) $(AR) d $@ pure.o
	$(Q) $(AR) d $@ vterminate.o
	$(Q) $(AR) d $@ guard.o
	$(Q) $(AR) d $@ functexcept.o
	$(Q) $(AR) d $@ del_op.o
	$(Q) $(AR) d $@ del_ops.o
	$(Q) $(AR) d $@ del_opv.o
	$(Q) $(AR) d $@ new_op.o
	$(Q) $(AR) d $@ new_opv.o

COMPONENT_TARGETS += $(LIBSTDCPP_DST)

EXTRA_LIBS += stdc++

endif
