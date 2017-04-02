default: build-deps
	$(MAKE) -C ./src
	$(MAKE) -C ./build

debug: build-deps
	$(MAKE) -C ./src -f Makefile debug
	$(MAKE) -C ./build

make-dirs:
	if [ ! -d "./include" ]; then mkdir include; fi

build-deps: make-dirs
	$(MAKE) -C ./dependencies

clean-deps:
	$(MAKE) -C ./dependencies -f Makefile clean

clean:
	rm -rf ./include
	find ./build/ -type f ! \( -iname "makefile" -or -iname "*.mp4" -or -iname "*.sofa" -or -iname "*.wav" \) -delete
