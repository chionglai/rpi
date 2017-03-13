################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/util/subdir.mk
-include src/module/st7735-lcd/subdir.mk
-include src/module/mpu6050-accelGyro/subdir.mk
-include src/module/mfrc522-rfid/subdir.mk
-include src/module/max7219-dotMatrix/subdir.mk
-include src/module/hmc5883-compass/subdir.mk
-include src/module/ds1307-rtc/subdir.mk
-include src/module/at24c32-eeprom/subdir.mk
-include src/math/subdir.mk
-include src/hw/bcm2835/subdir.mk
-include src/hw/subdir.mk
-include src/dsp/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: liblcc-rpi.a

# Tool invocations
liblcc-rpi.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Archiver'
	arm-linux-gnueabihf-ar -r  "liblcc-rpi.a" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(ARCHIVES)$(C_DEPS) liblcc-rpi.a
	-@echo ' '

post-build:
	-@echo 'Copy to lib folder'
	-bash /Users/chiong/work/c/lcclib/proj/lcc-rpi/../../script/install_lib.sh /Users/chiong/work/c/lcclib/proj/lcc-rpi/../../lib/
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
