LIB_FILENAME=../Usb.a
OBJ_FILES=\
	UsbInitialise.o \
	UsbDescriptors.o \
	UsbConfiguration.o \
	UsbOnOutTransaction.o \
	UsbOnInTransaction.o \
	UsbMstackCallbacks.o \
	UsbMstackHidCallbacks.o \
	UsbRtosIsr.o \
	UsbSmallReport.o \
	UsbEndpointInterfaceMapping.o \
	I2c.a \
	Spi.a \
	Core.a \
	Usb.a \
	Timer/UsbTimerDescriptors.o \
	Timer/UsbTimerReports.o \
	Ccp/UsbCcpDescriptors.o \
	Ccp/UsbCcpReports.o \
	Uart/UsbUartDescriptors.o \
	Uart/UsbUartReports.o \
	Adc/UsbAdcDescriptors.o \
	Adc/UsbAdcReports.o \
	Dac/UsbDacDescriptors.o \
	Dac/UsbDacReports.o \
	Comparator/UsbComparatorDescriptors.o \
	Comparator/UsbComparatorReports.o \
	Clc/UsbClcDescriptors.o \
	Clc/UsbClcReports.o \
	m-stack/src/usb.o \
	m-stack/src/usb_winusb.o \
	m-stack/src/usb_hid.o

CFLAGS:=$(filter-out -pedantic-errors,$(CFLAGS)) -I$(CURDIR) -I$(CURDIR)/m-stack/include

ARCHIVES=$(filter %.a, $(OBJ_FILES))

all: $(LIB_FILENAME)

.PHONY: clean
clean: subdirectories-clean
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Timer && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Ccp && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Uart && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Adc && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Dac && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Comparator && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd Clc && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd m-stack/src && $(RM) -f $(BUILD_ARTEFACT_PATTERN)

.PHONY: subdirectories-clean
subdirectories-clean:
	$(foreach dir, $(ARCHIVES), $(MAKE) -C $(patsubst %.a, %, $(dir)) clean && ) true

.PHONY: subdirectories
subdirectories:
	$(foreach dir, $(ARCHIVES), $(MAKE) -C $(patsubst %.a, %, $(dir)) && ) true

$(LIB_FILENAME): $(OBJ_FILES) subdirectories
	rm -f $(LIB_FILENAME) && $(AR) rcsT $(LIB_FILENAME) $(OBJ_FILES) && echo -e 'create $(LIB_FILENAME)\naddlib $(LIB_FILENAME)\nsave\nend' | $(AR) -M

include $(MAKEFILE_RULES)
