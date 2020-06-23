LIB_FILENAME=../Fusb303.a
OBJ_FILES=\
	Fusb303Initialise.o \
	Fusb303RtosIsr.o \
	Fusb303Isr.o \
	Fusb303Task.o \
	Fusb303ReadWrite.o \
	Fusb303BroadcastFlags.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
