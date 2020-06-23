LIB_FILENAME=../BenchTesting.a
OBJ_FILES=\
	BenchTestingInitialise.o \
	BenchTestingTask.o \
	BenchTestingTogglePins.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
