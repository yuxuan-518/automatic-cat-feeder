# Optional local build; PlatformIO is the primary project configuration.
AVR_BIN ?= $(HOME)/.platformio/packages/toolchain-atmelavr/bin
AVR_CC := $(AVR_BIN)/avr-gcc
AVR_OBJCOPY := $(AVR_BIN)/avr-objcopy
AVR_SIZE := $(AVR_BIN)/avr-size
HOST_CC ?= cc
SOURCES := $(wildcard src/*.c src/command/*.c)
CFLAGS := -mmcu=atmega2560 -DF_CPU=16000000UL -std=gnu11 -Os -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Iinclude

.PHONY: all test
all: build/firmware.hex

build/firmware.elf: $(SOURCES) $(wildcard include/*.h include/command/*.h)
	@mkdir -p build
	$(AVR_CC) $(CFLAGS) $(SOURCES) -Wl,--gc-sections -o $@
	$(AVR_SIZE) -C --mcu=atmega2560 $@

build/firmware.hex: build/firmware.elf
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@

test:
	@mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -Iinclude tests/test_controller.c src/motor.c src/demo.c src/command/command.c src/command/command_motor.c -o build/test_controller
	./build/test_controller
