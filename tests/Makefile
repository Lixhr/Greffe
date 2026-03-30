ARCH ?= thumb

.PHONY: all build test clean

all: build test

build:
	$(MAKE) -C bin/$(ARCH)

test: build
	python3 -m pytest cli/$(ARCH).py 
# 	make -C bin/$(ARCH)/ run-patched
