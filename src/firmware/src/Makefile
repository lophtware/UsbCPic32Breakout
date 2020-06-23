export DATE_YYYYMMDD=`date +"%Y%m%d"`
export FIRMWARE_VERSION="$(DATE_YYYYMMDD)"

ELF_FILENAME=UsbCPic32Breakout.elf
MAP_FILENAME=UsbCPic32Breakout.map
HEX_FILENAME=UsbCPic32Breakout.hex

OBJ_FILES=\
	Main.o \
	ConfigWords.o \
	Syskey.o \
	Fault.o \
	Oscillator.o \
	Configuration.o \
	Crc32.o \
	UnlockKey.o \
	Nvm.o \
	DeviceInfoI2cBank.o \
	Dma.o \
	BenchTesting.a \
	Usb.a \
	Fusb303.a \
	I2c.a \
	Spi.a \
	Pins.a \
	FreeRtos.a

ARCHIVES=$(filter %.a, $(OBJ_FILES))

all: $(HEX_FILENAME)

.PHONY: install
install: all
	$(PICKIT3_IPE) -P32mm0256gpm028 -OL -M -Y -F$(HEX_FILENAME)

.PHONY: cicd
cicd: all
	$(CICD_CP) $(HEX_FILENAME) $(CICD_DESTINATION)

.PHONY: clean
clean: subdirectories-clean
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

.PHONY: subdirectories-clean
subdirectories-clean:
	$(foreach dir, $(ARCHIVES), $(MAKE) -C $(patsubst %.a, %, $(dir)) clean && ) true

.PHONY: subdirectories
subdirectories:
	$(foreach dir, $(ARCHIVES), $(MAKE) -C $(patsubst %.a, %, $(dir)) && ) true

$(HEX_FILENAME): $(ELF_FILENAME)
	$(BIN2HEX) $(BIN2HEXFLAGS) $(ELF_FILENAME)

$(ELF_FILENAME): $(filter %.o, $(OBJ_FILES)) subdirectories
	$(LD) $(LDFLAGS) -Xlinker -Map=$(MAP_FILENAME) -o $(ELF_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
