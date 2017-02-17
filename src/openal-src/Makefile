default:
	$(MAKE) -C ./example

install-deps:
	$(MAKE) -C ./dependencies -f Makefile clean
	$(MAKE) -C ./dependencies -f Makefile install-deps

clean:
	$(MAKE) -C ./example -f Makefile clean
	$(MAKE) -C ./dependencies -f Makefile clean
