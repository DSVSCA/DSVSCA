default:
	$(MAKE) -C ./src

make-deps:
	$(MAKE) -C ./src -f Makefile make-deps

install-deps:
	$(MAKE) -C ./src -f Makefile install-deps

openal:
	$(MAKE) -C ./src -f Makefile openal

clean:
	$(MAKE) -C ./src -f Makefile clean
