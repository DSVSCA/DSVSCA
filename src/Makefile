default:
	$(MAKE) -C ./input
	$(MAKE) -C ./openal-src
	$(MAKE) -C ./virtualizer

openal:
	$(MAKE) -C ./openal-src

clean:
	$(MAKE) -C ./input -f Makefile clean
	$(MAKE) -C ./openal-src -f Makefile clean
	$(MAKE) -C ./virtualizer -f Makefile clean
