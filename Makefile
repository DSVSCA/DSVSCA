default: build-deps
	$(MAKE) -C ./src

make-dirs:
	if [ ! -d "./build" ]; then mkdir build; fi
	if [ ! -d "./include" ]; then mkdir include; fi

build-deps: make-dirs
	$(MAKE) -C ./dependencies

clean-deps:
	$(MAKE) -C ./dependencies -f Makefile clean

clean:
	rm -rf ./include
	rm -rf ./build
