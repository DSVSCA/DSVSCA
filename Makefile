UNAME := $(shell uname)

default: build-deps
	$(shell g++ -v)
	$(MAKE) -C ./src
ifeq ($(UNAME),Darwin)
	$(MAKE) darwin -C ./build
endif
ifeq ($(UNAME),Linux)
	$(MAKE) -C ./build	
endif
	$(MAKE) -C ./example

debug: build-deps
	$(MAKE) -C ./src -f Makefile debug
	$(MAKE) -C ./build
	$(MAKE) -C ./example -f Makefile debug

ui:
	$(MAKE) -C ./src -f Makefile ui-default
	$(MAKE) -C ./build -f Makefile ui
	$(MAKE) -C ./example -f Makefile ui

ui-debug:
	$(MAKE) -C ./src -f Makefile ui-debug
	$(MAKE) -C ./build -f Makefile ui
	$(MAKE) -C ./example -f Makefile ui

make-dirs:
	if [ ! -d "./include" ]; then mkdir include; fi

build-deps: make-dirs
ifeq ($(UNAME),Darwin)
	$(MAKE) darwin -C ./dependencies
endif
ifeq ($(UNAME),Linux)
	$(MAKE) -C ./dependencies
endif

clean-deps:
	$(MAKE) -C ./dependencies -f Makefile clean

clean:
	rm -rf ./include
	find ./build/ -mindepth 1 ! \( -iname "makefile" -or -iname "*.mp4" -or -iname "*.sofa" -or -iname "*.wav" \) -delete
	find ./example/ -mindepth 1 ! \( -iname "makefile" -or -iname "*.mp4" -or -iname "*.sofa" -or -iname "*.wav" -or -iname "*.cpp" -or -iname "*.h" \) -delete
