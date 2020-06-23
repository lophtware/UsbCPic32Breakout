LIB_FILENAME=../Core.a
OBJ_FILES=\
	UsbCoreDescriptors.o \
	UsbCoreInitialise.o \
	UsbCorePinsTask.o \
	UsbCoreReports.o \
	UsbCoreReportsTask.o \
	UsbCoreStatusReport.o \
	UsbCoreSetUnlockKeyReport.o \
	UsbCoreStoreConfigurationReport.o \
	UsbCorePinConfigurationReport.o \
	UsbCorePinLatAndStatusReports.o \
	UsbCorePinsChangedResetReport.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
