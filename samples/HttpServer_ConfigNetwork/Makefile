#####################################################################
#### Please don't change this file. Use Makefile-user.mk instead ####
#####################################################################
# Including user Makefile.
# Should be used to set project-specific parameters
include ./Makefile-user.mk

ifndef SMING_HOME
$(error SMING_HOME is not set. Please configure it as an environment variable, or in Makefile-user.mk)
endif

# Include application Makefile
include $(SMING_HOME)/Makefile-app.mk
