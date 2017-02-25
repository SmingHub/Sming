#	Windows.mk
#
#==============================================================================
# 	default directories (make compatible format /<drive>/<absolute path>)
#------------------------------------------------------------------------------
# 	ESP8266 SDK directories
ESP_HOME 		?= c:/Espressif
ESP_HOME 		:= $(subst \,/,$(addprefix /,$(subst :,,$(ESP_HOME))))

SDK_BASE        := $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS       := $(ESP_HOME)/utils/ESP8266

#==============================================================================
# 	default serial settings
#------------------------------------------------------------------------------
COM_PORT		?= COM3

#==============================================================================
# 	tool settings
#------------------------------------------------------------------------------
ESPTOOL		 	?= $(SDK_TOOLS)/esptool.exe
GET_FILESIZE 	?= stat --printf="%s"
MEMANALYZER  	?= $(SDK_TOOLS)/memanalyzer.exe $(OBJDUMP).exe

#	see http://www.horstmuc.de/wbat32.htm#wait
PAUSE5			?= wait.exe 5 CR 

#==============================================================================
# 	serial terminal settings
#------------------------------------------------------------------------------
#	use plink.exe and allow ansi escape sequences
#	see http://www.putty.org/ and https://github.com/adoxa/ansicon 
TERMINAL    	?= start ansicon plink -serial $(COM_PORT) -sercfg $(COM_SPEED_SERIAL),8,n,1
KILL_TERM   	?= taskkill -f -im plink.exe  2>nul
