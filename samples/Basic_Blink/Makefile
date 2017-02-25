#	Makefile
#
#==============================================================================
#	project settings 
#	see sming.mk for more settings
#------------------------------------------------------------------------------
#	flash size (required)
SPI_SIZE        = 4M

#	your source directories (default: app)
MODULES         = app 

#	your extra include directies (default: none)
EXTRA_INCDIR    = 

#	your extra libraries (default: none)
EXTRA_LIBS      = 

#	spiff size (default: 0)
SPIFF_SIZE		= 0

#	reserved flash area for user data (default: 0)
USER_FLASH_SIZE	= 0

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
