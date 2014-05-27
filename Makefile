###########################################################################
# Makefile for ViamSonus.
# Author: J. Ian Lindsay
###########################################################################
SHELL          = /bin/sh
WHO_I_AM       = $(shell whoami)
HOME_DIRECTORY = /home/$(WHO_I_AM)



###########################################################################
# Variables for the host compilation...
###########################################################################
CC       = gcc
CFLAGS   = -Wall
CXXFLAGS = -std=gnu++11
LIBS	= -lstdc++


###########################################################################
# Variables for the firmware compilation...
###########################################################################
CPU_SPEED          = 48000000
TOOLCHAIN          = $(HOME_DIRECTORY)/arduino/hardware/tools/arm-none-eabi/bin
CC_CROSS           = $(TOOLCHAIN)/arm-none-eabi-g++
CC_CROSS_FLAGS     = -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -nostdlib -DTEENSYDUINO=117 -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH
CC_CROSS_CPU_FLAGS = -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -mthumb -D__MK20DX256__
CC_CROSS_INCLUDES  = -I./ -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 -I$(HOME_DIRECTORY)/arduino/libraries/EEPROM -I$(HOME_DIRECTORY)/arduino/libraries/SPI -I$(HOME_DIRECTORY)/arduino/libraries/
OBJCOPY            = $(TOOLCHAIN)/arm-none-eabi-objcopy
LD_CROSS           = $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/mk20dx256.ld
FORMAT             = ihex
BUILD_TEMP_PATH    = ./build.tmp
SOURCE_FILE_LIST   = Logger/*.cpp i2c-adapter/*.cpp AudioRouter/*.cpp ISL23345/*.cpp ADG2128/*.cpp 
TEENSY_LOADER_PATH = $(HOME_DIRECTORY)/arduino/hardware/tools

###########################################################################
# Rules for building the host-side follow...
###########################################################################
default:	audioroute

debug:		audioroute.o
	$(CC) $(CFLAGS) -ggdb -g -o audioroute audioroute.o $(LIBS)
			
audioroute:	audioroute.o
	$(CC) $(CFLAGS) -o audioroute *.o $(LIBS)

audioroute.o:	ViamSonus.cpp
	$(CC) $(CXXFLAGS) $(CFLAGS) -c ViamSonus.cpp $(SOURCE_FILE_LIST) -fno-exceptions

install: audioroute
	cp audioroute /usr/bin/audioroute



###########################################################################
# Rules for building the firmware follow...
###########################################################################
micro: ViamSonus.elf

micro-dep:
	mkdir -p $(BUILD_TEMP_PATH)
	$(CC_CROSS) $(CC_CROSS_FLAGS) $(CC_CROSS_CPU_FLAGS) $(CC_CROSS_INCLUDES) ViamSonus.cpp $(SOURCE_FILE_LIST) > $(BUILD_TEMP_PATH)/$*.d

ViamSonus.elf: micro-dep teensy3-support
	$(CC_CROSS) $(CC_CROSS_FLAGS) $(CC_CROSS_CPU_FLAGS) $(CC_CROSS_INCLUDES) ViamSonus.cpp $(SOURCE_FILE_LIST) 
	mv *.o $(BUILD_TEMP_PATH)
	mv *.d $(BUILD_TEMP_PATH)
	$(CC_CROSS) -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(LD_CROSS) -o $(BUILD_TEMP_PATH)/ViamSonus.elf $(BUILD_TEMP_PATH)/*.o -L$(BUILD_TEMP_PATH) -larm_cortexM4l_math -lm 



###########################################################################
# Create final output files (.hex, .eep) from ELF output file.
###########################################################################
ViamSonus.hex: ViamSonus.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $(BUILD_TEMP_PATH)/ViamSonus.elf $(BUILD_TEMP_PATH)/ViamSonus.eep 
	$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .lock -R .signature $(BUILD_TEMP_PATH)/ViamSonus.elf ViamSonus.hex



program: ViamSonus.hex
	$(TEENSY_LOADER_PATH)/teensy_loader_cli -mmcu=mk20dx128 -w -v ViamSonus.hex



teensy3-support:
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 -I$(HOME_DIRECTORY)/arduino/libraries/Time -I$(HOME_DIRECTORY)/arduino/libraries/SPI -I$(HOME_DIRECTORY)/arduino/libraries/Time/utility $(HOME_DIRECTORY)/arduino/libraries/Time/DateStrings.cpp -o $(BUILD_TEMP_PATH)/DateStrings.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 -I$(HOME_DIRECTORY)/arduino/libraries/Time -I$(HOME_DIRECTORY)/arduino/libraries/SPI -I$(HOME_DIRECTORY)/arduino/libraries/Time/utility $(HOME_DIRECTORY)/arduino/libraries/Time/Time.cpp -o $(BUILD_TEMP_PATH)/Time.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 -I$(HOME_DIRECTORY)/arduino/libraries/Time -I$(HOME_DIRECTORY)/arduino/libraries/SPI -I$(HOME_DIRECTORY)/arduino/libraries/SPI/utility $(HOME_DIRECTORY)/arduino/libraries/SPI/SPI.cpp -o $(BUILD_TEMP_PATH)/SPI.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467113 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_dev.c -o $(BUILD_TEMP_PATH)/usb_dev.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_seremu.c -o $(BUILD_TEMP_PATH)/usb_seremu.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/keylayouts.c -o $(BUILD_TEMP_PATH)/keylayouts.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/serial2.c -o $(BUILD_TEMP_PATH)/serial2.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/analog.c -o $(BUILD_TEMP_PATH)/analog.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_serial.c -o $(BUILD_TEMP_PATH)/usb_serial.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_desc.c -o $(BUILD_TEMP_PATH)/usb_desc.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/eeprom.c -o $(BUILD_TEMP_PATH)/eeprom.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_keyboard.c -o $(BUILD_TEMP_PATH)/usb_keyboard.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/serial1.c -o $(BUILD_TEMP_PATH)/serial1.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_mouse.c -o $(BUILD_TEMP_PATH)/usb_mouse.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_mem.c -o $(BUILD_TEMP_PATH)/usb_mem.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/math_helper.c -o $(BUILD_TEMP_PATH)/math_helper.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_midi.c -o $(BUILD_TEMP_PATH)/usb_midi.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/serial3.c -o $(BUILD_TEMP_PATH)/serial3.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_rawhid.c -o $(BUILD_TEMP_PATH)/usb_rawhid.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_joystick.c -o $(BUILD_TEMP_PATH)/usb_joystick.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467114 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/yield.c -o $(BUILD_TEMP_PATH)/yield.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467115 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/pins_teensy.c -o $(BUILD_TEMP_PATH)/pins_teensy.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467115 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/mk20dx128.c -o $(BUILD_TEMP_PATH)/mk20dx128.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467115 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/touch.c -o $(BUILD_TEMP_PATH)/touch.c.o 
	$(TOOLCHAIN)/arm-none-eabi-gcc -c -g -Os -Wall -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -DTIME_T=1398467115 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/nonstd.c -o $(BUILD_TEMP_PATH)/nonstd.c.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/i2c_t3.cpp -o $(BUILD_TEMP_PATH)/i2c_t3.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/Print.cpp -o $(BUILD_TEMP_PATH)/Print.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/WString.cpp -o $(BUILD_TEMP_PATH)/WString.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/WMath.cpp -o $(BUILD_TEMP_PATH)/WMath.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_flightsim.cpp -o $(BUILD_TEMP_PATH)/usb_flightsim.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/usb_inst.cpp -o $(BUILD_TEMP_PATH)/usb_inst.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/Tone.cpp -o $(BUILD_TEMP_PATH)/Tone.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/Stream.cpp -o $(BUILD_TEMP_PATH)/Stream.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/avr_emulation.cpp -o $(BUILD_TEMP_PATH)/avr_emulation.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/new.cpp -o $(BUILD_TEMP_PATH)/new.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/Encoder/Encoder.cpp -o $(BUILD_TEMP_PATH)/Encoder.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/IPAddress.cpp -o $(BUILD_TEMP_PATH)/IPAddress.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/rbuf/rbuf.cpp -o $(BUILD_TEMP_PATH)/rbuf.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/HardwareSerial1.cpp -o $(BUILD_TEMP_PATH)/HardwareSerial1.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/HardwareSerial3.cpp -o $(BUILD_TEMP_PATH)/HardwareSerial3.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/main.cpp -o $(BUILD_TEMP_PATH)/main.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/IntervalTimer.cpp -o $(BUILD_TEMP_PATH)/IntervalTimer.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/HardwareSerial2.cpp -o $(BUILD_TEMP_PATH)/HardwareSerial2.cpp.o 
	$(TOOLCHAIN)/arm-none-eabi-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -mcpu=cortex-m4 -DF_CPU=$(CPU_SPEED) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -mthumb -nostdlib -D__MK20DX256__ -DTEENSYDUINO=117 -I$(HOME_DIRECTORY)/working-copies/ViamSonus/trunk -fno-rtti -felide-constructors -std=gnu++0x -DUSB_SERIAL -DLAYOUT_US_ENGLISH -I$(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3 $(HOME_DIRECTORY)/arduino/hardware/teensy/cores/teensy3/AudioStream.cpp -o $(BUILD_TEMP_PATH)/AudioStream.cpp.o 


###########################################################################
# These rules are common to both builds...
###########################################################################
clean:	
	rm -rf audioroute *.o *~ *.d *.hex $(BUILD_TEMP_PATH)

