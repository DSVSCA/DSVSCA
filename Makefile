default:
	if [ ! -d "./build" ]; then mkdir build; fi
	$(MAKE) -C ./src

install-deps:
	$(MAKE) -C ./src -f Makefile install-deps

openal:
	$(MAKE) -C ./src -f Makefile openal

clean:
	$(MAKE) -C ./src -f Makefile clean
