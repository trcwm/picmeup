# configuration
# https://www.engbedded.com/conffuse/

PRJ=wiggler
MCU=atmega328p
AVRDUDE_MCU=m328p

DATESTR := $(shell date +"%d-%m-%Y %X")
VERSIONSTRING := "WIGGLER v1.0 $(DATESTR)"

##################################################
# setup toolchain and AVRDUDE
##################################################

TOOLCHAIN=/opt/avr-gcc_mc/bin
PORT=/dev/ttyUSB0
AVRDUDE=/usr/bin/avrdude -c arduino -p $(AVRDUDE_MCU)

##################################################
# setup executables
##################################################

CC=$(TOOLCHAIN)/avr-gcc
SIZE=$(TOOLCHAIN)/avr-size --format=avr --mcu=$(MCU)
OBJCOPY=$(TOOLCHAIN)/avr-objcopy
OBJDUMP=$(TOOLCHAIN)/avr-objdump

##################################################
# compiler and linker flags
##################################################

CFLAGS=-g -Wall -mcall-prologues -mmcu=$(MCU) -Os -DF_CPU=16000000 -std=gnu11
LDFLAGS=-Wl,-gc-sections -Wl,-relax

##################################################
# setup source files
##################################################

SRC       = version.h version.c version.c.in atmega328/uart.h atmega328/uart.c atmega328/timer.h atmega328/timer.c $(PRJ).c
CFILES    = $(filter %.c, $(SRC))

EXTC     := $(foreach dir, $(EXT), $(wildcard $(dir)/*.c))
OBJ       = $(CFILES:.c=.o) $(EXTC:.c=.o)


##################################################
# create pre and main build targets
##################################################

all: pre-build main-build

# use version.c.in to create version.c
pre-build: version.c.in
	sed 's/@@VERSIONSTRING@@/$(VERSIONSTRING)/g' version.c.in >version.c

main-build:
	@$(MAKE) --no-print-directory $(PRJ).hex

##################################################
# compile c files
##################################################
.c.o: 
	$(CC) $(CFLAGS) -c $< -o $@

# link it
${PRJ}.elf: $(OBJ)
	$(CC) $(CFLAGS) -o $(PRJ).elf $(OBJ)

##################################################
# generate hex file
##################################################
$(PRJ).hex: $(PRJ).elf
	$(info $(VERSIONSTRING))
	rm -f $(PRJ).hex
	$(OBJCOPY) -R .eeprom -O ihex $(PRJ).elf $(PRJ).hex
	$(SIZE) $(PRJ).elf

##################################################
# generate disassembly files for debugging
##################################################
disasm: $(PRJ).elf
	$(OBJDUMP) -d $(PRJ).elf

##################################################
# check avrdude connectivity
##################################################
avrdude: 
	${AVRDUDE} -v -P $(PORT)

##################################################
# program the uC
##################################################
program: $(PRJ).hex
	${AVRDUDE} -P $(PORT) -U flash:w:$(PRJ).hex

##################################################
# clean up this mess
##################################################
clean:
	rm -f *.o *.hex *.obj *.hex *.elf
	rm version.c
