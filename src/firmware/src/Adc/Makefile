LIB_FILENAME=../Adc.a
OBJ_FILES=\
	AdcInitialise.o \
	AdcTask.o \
	AdcTransaction.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
