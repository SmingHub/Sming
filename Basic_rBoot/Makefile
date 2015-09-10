#####################################################################
#### Please don't change this file. Use Makefile-user.mk instead ####
#####################################################################
# Including user Makefile.
# Should be used to set project-specific parameters
include ./Makefile-user.mk

# Important parameters check.
# We need to make sure SMING_HOME and ESP_HOME variables are set.
# You can use Makefile-user.mk in each project or use enviromental variables to set it globally.
 
ifndef SMING_HOME
$(error SMING_HOME is not set. Please configure it in Makefile-user.mk)
endif
ifndef ESP_HOME
$(error ESP_HOME is not set. Please configure it in Makefile-user.mk)
endif

# Include main Sming Makefile
ifeq ($(RBOOT_ENABLED), 1)
include $(SMING_HOME)/Makefile-rboot.mk
else
include $(SMING_HOME)/Makefile-project.mk
endif
