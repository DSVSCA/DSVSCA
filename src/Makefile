default:
	$(MAKE) -C ./sjoin
	$(MAKE) -C ./input
	$(MAKE) -C ./openal-src
install-deps:
	$(MAKE) -C ./openal-src -f Makefile install-deps

openal:
	$(MAKE) -C ./openal-src

clean:
	$(MAKE) -C ./input -f Makefile clean
	$(MAKE) -C ./openal-src -f Makefile clean
