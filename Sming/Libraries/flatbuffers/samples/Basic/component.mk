COMPONENT_DEPENDS := flatbuffers

CUSTOM_TARGETS	+= include/monster.h

include/monster.h: ../../src/samples/monster_generated.h
	$(Q) cp $< $@