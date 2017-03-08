#	Makefile
#
#==============================================================================
#	project settings (see sming.mk)
#------------------------------------------------------------------------------
#	flash size (required)
SPI_SIZE        = 4M

#==============================================================================
#	now let sming do all the work ...
#------------------------------------------------------------------------------
#	make sure that SMING_HOME is set and the path compatible with make 
ifndef SMING_HOME
	$(error please set SMING_HOME)
endif
SMING_HOME 		:= $(subst \,/,$(addprefix /,$(subst :,,$(SMING_HOME))))

include $(SMING_HOME)/sming.mk

#==============================================================================
#	add your own targets below
#	e. g. for special prerequsisites or ota deployment 
#------------------------------------------------------------------------------
