LIB_FILENAME=../Spi.a
OBJ_FILES=\
	UsbSpiDescriptors.o \
	UsbSpiInitialise.o \
	UsbSpiPinAssignment.o \
	UsbSpiReports.o \
	UsbSpiReportsTask.o \
	UsbSpiSlaveConfigurationReport.o \
	UsbSpiTransactionReport.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
