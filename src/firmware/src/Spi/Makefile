LIB_FILENAME=../Spi.a
OBJ_FILES=\
	SpiInitialise.o \
	SpiConfiguration.o \
	SpiSlaves.o \
	SpiTask.o \
	SpiMaster.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
