export BASE_DIR=$(CURDIR)
export FIRMWARE_DIR=$(BASE_DIR)/src
export MAKEFILE_RULES=$(BASE_DIR)/Makefile.rules
export XC32_DIR=/opt/microchip/xc32/v2.30/bin
export MPLABX_DIR=/opt/microchip/mplabx/v5.05
export MAKE=make
export FREERTOS_INCLUDES=-I$(BASE_DIR)/src/FreeRtos -I$(BASE_DIR)/src/FreeRtos/FreeRtos/include -I$(BASE_DIR)/src/FreeRtos/FreeRtos/portable/MPLAB/PIC32MM
# export DEFINES=-DENABLE_BENCH_TESTING
export CFLAGS_COMMON=-mprocessor=32MM0256GPM028 -mreserve=data@0x00000000:0x000001ff -mno-float -g3 -O2 -std=c99 -pedantic-errors -Wall -fstack-usage $(FREERTOS_INCLUDES) $(DEFINES)
export CC=$(XC32_DIR)/xc32-gcc
export CFLAGS=$(CFLAGS_COMMON) -c
export AS=$(XC32_DIR)/xc32-gcc
export ASFLAGS=$(CFLAGS_COMMON) -c
export AR=$(XC32_DIR)/xc32-ar
export ARFLAGS=rcs
export LD=$(XC32_DIR)/xc32-gcc
export LDFLAGS=$(CFLAGS_COMMON) -Xlinker --report-mem
export BIN2HEX=$(XC32_DIR)/xc32-bin2hex
export BIN2HEXFLAGS=-omf=elf
export BUILD_ARTEFACT_PATTERN=*.hex *.elf *.map *.lst *.o *.a *.su *.dfinish *.log
export PICKIT3_IPE=$(MPLABX_DIR)/mplab_platform/mplab_ipe/ipecmd.sh -TPPK3
export CICD_CP=scp
export CICD_DESTINATION=pi@porkpi:/home/pi/usbc-pic32-breakout

all:
	$(MAKE) -C src all

install:
	$(MAKE) -C src install

cicd:
	$(MAKE) -C src cicd

clean:
	$(MAKE) -C src clean
