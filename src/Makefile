default:
	$(MAKE) -C ./input
	$(MAKE) -C ./openal-src

make-deps:
	$(MAKE) -C ./openal-src -f Makefile make-deps

install-deps:
	$(MAKE) -C ./openal-src -f Makefile install-deps

openal:
	$(MAKE) -C ./openal-src

clean:
	$(MAKE) -C ./input -f Makefile clean
	$(MAKE) -C ./openal-src -f Makefile clean
