LIB_FILENAME=../I2c.a
OBJ_FILES=\
	I2cInitialise.o \
	I2cConfiguration.o \
	I2cTask.o \
	I2cMaster.o \
	I2cSlave.o \
	I2cRam.o \
	I2cRom.o \
	I2cIsr.o \
	I2cRtosIsr.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
