LIB_FILENAME=../Pins.a
OBJ_FILES=\
	PinsConfiguration.o \
	PinsMaskedLat.o \
	PinsChangedIsr.o \
	PinsChangedRtosIsr.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
