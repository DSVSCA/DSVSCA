default: build-deps
	$(MAKE) -C ./src
	$(MAKE) -C ./build
	$(MAKE) -C ./example

debug: build-deps
	$(MAKE) -C ./src -f Makefile debug
	$(MAKE) -C ./build
	$(MAKE) -C ./example -f Makefile debug

make-dirs:
	if [ ! -d "./include" ]; then mkdir include; fi

build-deps: make-dirs
	$(MAKE) -C ./dependencies

clean-deps:
	$(MAKE) -C ./dependencies -f Makefile clean

clean:
	rm -rf ./include
	find ./build/ -mindepth 1 ! \( -iname "makefile" -or -iname "*.mp4" -or -iname "*.sofa" -or -iname "*.wav" \) -delete
	find ./example/ -mindepth 1 ! \( -iname "makefile" -or -iname "*.mp4" -or -iname "*.sofa" -or -iname "*.wav" -or -iname "*.cpp" -or -iname "*.h" \) -delete
