LIB_FILENAME=../UsbCc.a
OBJ_FILES=\
	UsbCcStubs.o \
	Fusb303.a \
	UsbCcAdc.a

ARCHIVES=$(filter %.a, $(OBJ_FILES))

all: $(LIB_FILENAME)

.PHONY: clean
clean: subdirectories-clean
	$(RM) -f $(BUILD_ARTEFACT_PATTERN)

.PHONY: subdirectories-clean
subdirectories-clean:
	$(foreach dir, $(ARCHIVES), $(MAKE) -C $(patsubst %.a, %, $(dir)) clean && ) true

.PHONY: subdirectories
subdirectories:
	$(foreach dir, $(ARCHIVES), $(MAKE) -C $(patsubst %.a, %, $(dir)) && ) true

$(LIB_FILENAME): $(OBJ_FILES) subdirectories
	rm -f $(LIB_FILENAME) && $(AR) rcsT $(LIB_FILENAME) $(OBJ_FILES) && echo -e 'create $(LIB_FILENAME)\naddlib $(LIB_FILENAME)\nsave\nend' | $(AR) -M

include $(MAKEFILE_RULES)
