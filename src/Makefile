# set the following variables 
# path to Arduino IDE
# PATH_TO_ARDUINO_IDE = /path/to/your/arduinoIDE
# Arduino IDE version
# ARDUINO_IDE_VER = 1.6.X
# local github mirror, if applicable, https://github.com is used per default
# GITHUB_MIRROR = /path/to/your/github mirror

ARDUINO_DIR = $(PATH_TO_ARDUINO_IDE)/arduino-$(ARDUINO_IDE_VER)

ifneq ("$(wildcard $(GITHUB_MIRROR))","")
GITHUB = $(GITHUB_MIRROR)
else
GITHUB = https://github.com
endif

.PHONY: default git_setup clean_all clean_builds minicom

# other info is taken from boards.txt since we use the UNO standard board here
BOARD_TAG=uno
MONITOR_BAUDRATE=115200

CPPFLAGS += -DMONITOR_BAUDRATE=$(MONITOR_BAUDRATE) 

#default target
default: git_setup
######################################################################################
# setup section
#local library directories
ARDMK_DIR := ./local-libs/Arduino-Makefile

USER_LIB_PATH := ./local-libs
ARDUINO_LIBS = SPI Wire MultiLCD/MicroLCD xMAX6675 Arduino-PID-Library 


XMAX6675_DIR := $(USER_LIB_PATH)/xMAX6675 
PID_DIR :=  $(USER_LIB_PATH)/Arduino-PID-Library
LCD_DIR :=  $(USER_LIB_PATH)/MultiLCD

LOCAL_GIT_LIBRARIES = $(ARDMK_DIR) $(XMAX6675_DIR) $(PID_DIR) $(LCD_DIR)

# local library includes
# Arduino Makefile environment
ifneq ("$(wildcard $(ARDMK_DIR))","")
include $(ARDMK_DIR)/Arduino.mk
endif


#setup part
git_setup: $(LOCAL_GIT_LIBRARIES)
	@echo Do the following:
	@echo make upload

#local library target/rules
$(ARDMK_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/sudar/Arduino-Makefile $@


$(XMAX6675_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/eharvester/xMAX6675 $@


$(PID_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/br3ttb/Arduino-PID-Library $@

$(LCD_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/stanleyhuangyc/MultiLCD $@

######################################################################################
#cleanup section
clean_all: clean_builds 
	@echo "Cleaning local libs ..."
	@rm -rf ./local-libs

clean_builds: 
	@echo "Cleaning builds ..."
	@rm -rf ./build-*


#other PHONY stuff
minicom:
	gnome-terminal --command='minicom -D $(DEVICE_PATH) -b $(MONITOR_BAUDRATE)' --title='$(DEVICE_PATH) $(MONITOR_BAUDRATE) baud'

