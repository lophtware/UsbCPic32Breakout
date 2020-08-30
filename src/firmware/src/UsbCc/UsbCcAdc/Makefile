LIB_FILENAME=../UsbCcAdc.a
OBJ_FILES=\
	UsbCcAdcInitialise.o \
	UsbCcAdcTask.o \
	UsbCcAdcDecode.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
