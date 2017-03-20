default:
	if [ ! -d "./build" ]; then mkdir build; fi
	$(MAKE) -C ./src

openal:
	$(MAKE) -C ./src -f Makefile openal

clean:
	$(MAKE) -C ./src -f Makefile clean
