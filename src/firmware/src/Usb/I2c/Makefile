LIB_FILENAME=../I2c.a
OBJ_FILES=\
	UsbI2cDescriptors.o \
	UsbI2cInitialise.o \
	UsbI2cPinAssignment.o \
	UsbI2cBank.o \
	UsbI2cReports.o \
	UsbI2cReportsAndEventsTask.o \
	UsbI2cBusConfigurationReport.o \
	UsbI2cMasterConfigurationReport.o \
	UsbI2cSlaveConfigurationReport.o \
	UsbI2cSlaveReportConfigurationReport.o \
	UsbI2cProtectedRamAddressMaskReport.o \
	UsbI2cRamBankWriteProtectFlagReport.o \
	UsbI2cRamInitialisationContentsReport.o \
	UsbI2cProtectedRomAddressMaskReport.o \
	UsbI2cRomBankWriteProtectFlagReport.o \
	UsbI2cRomContentsReport.o \
	UsbI2cRomAndRamTransactionReports.o \
	UsbI2cTransactionReport.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
