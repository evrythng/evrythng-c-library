PROJECT_DIR:=$(shell pwd)

ifeq ($(DEBUG),)
	DEBUG := 1
endif

ifeq ($(DEBUG),0)
	BUILD_TYPE := Release
else
	BUILD_TYPE := Debug
endif

ifeq ($(VERBOSE),)
	VERBOSE := 0
endif

ifeq ($(VERBOSE),0)
.SILENT:
else ifeq ($(VERBOSE),1)
	AT := @
else
endif

RMRF := rm -rf

ifeq ($(BUILD_DIR),)
	ifeq ($(wildcard Makefile), Makefile)
		BUILD_DIR := .
	else
		BUILD_DIR := build
		ifneq ($(DEBUG),0)
			BUILD_DIR := $(BUILD_DIR)_debug
		endif
	endif
endif

.PHONY: build_dir all cleanall makefile docs

all: build_dir
	$(MAKE) -C $(BUILD_DIR) -f Makefile all

ifeq ($(BUILD_DIR),.)

BUILD_SYSTEM_FILES := CMakeCache.txt CMakeFiles cmake_install.cmake \
    Makefile CMakeTimestamps

cleanall: clean distclean
distclean:
	for f in $(BUILD_SYSTEM_FILES); do\
		find $(BUILD_DIR) -name $$f | xargs -I{} $(RMRF) {} ;\
	done
else
cleanall: distclean
distclean:
	$(RMRF) $(BUILD_DIR)
	$(RMRF) docs/html
endif

build_dir:
	if [ ! -f $(BUILD_DIR)/CMakeCache.txt ] ; then \
		mkdir -p $(BUILD_DIR); \
		cd $(BUILD_DIR); \
		cmake $(PROJECT_DIR) -DCMAKE_BUILD_TYPE:STRING=$(BUILD_TYPE); \
	fi

%:: build_dir
	$(MAKE) -C $(BUILD_DIR) -f Makefile $@

docs:
	doxygen docs/Doxyfile

runtests: all
	-$(BUILD_DIR)/tests/test
