LIB_FILENAME=../Usb.a
OBJ_FILES=\
	UsbUsbDescriptors.o \
	UsbUsbReports.o \
	UsbUsbInitialise.o \
	UsbUsbI2cBank.o \
	UsbUsbReportsAndEventsTask.o \
	UsbUsbConfigurationNameDescriptorReport.o \
	UsbUsbPowerConfigurationReport.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
