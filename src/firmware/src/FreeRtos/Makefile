LIB_FILENAME=../FreeRtos.a
OBJ_FILES=\
	FreeRtosTimerConfiguration.o \
	FreeRtos/croutine.o \
	FreeRtos/event_groups.o \
	FreeRtos/list.o \
	FreeRtos/queue.o \
	FreeRtos/stream_buffer.o \
	FreeRtos/tasks.o \
	FreeRtos/timers.o \
	FreeRtos/portable/MemMang/heap_1.o \
	FreeRtos/portable/MPLAB/PIC32MM/port.o \
	FreeRtos/portable/MPLAB/PIC32MM/port_asm.o

all: $(LIB_FILENAME)

clean:
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd FreeRtos && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd FreeRtos/portable/MemMang && $(RM) -f $(BUILD_ARTEFACT_PATTERN)
	cd FreeRtos/portable/MPLAB/PIC32MM && $(RM) -f $(BUILD_ARTEFACT_PATTERN)

$(LIB_FILENAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(LIB_FILENAME) $(OBJ_FILES)

include $(MAKEFILE_RULES)
