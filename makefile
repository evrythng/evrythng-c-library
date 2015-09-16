.PHONY: docs gen_config clean

RMRF=rm -rf
PROJECT_DIR=$(shell pwd)

all: gen_config docs

clean:
	@$(RMRF) docs/html

docs:
	@doxygen docs/Doxyfile

gen_config:
	@$(PROJECT_DIR)/tests/gen_header.sh
