default: copy-headers
	$(MAKE) -C ./sjoin
	$(MAKE) -C ./input
	$(MAKE) -C ./virtualizer
	$(MAKE) -C ./encoder
	g++ -std=c++11 -c main.cpp -I../include/ -o ../build/main.o

debug: copy-headers
	$(MAKE) -C ./sjoin -f Makefile debug
	$(MAKE) -C ./input -f Makefile debug
	$(MAKE) -C ./virtualizer -f Makefile debug
	$(MAKE) -C ./encoder -f Makefile debug
	g++ -g -std=c++11 -c main.cpp -I../include/ -o ../build/main.o

copy-headers:
	$(MAKE) -C ./sjoin -f Makefile copy-headers
	$(MAKE) -C ./input -f Makefile copy-headers
	$(MAKE) -C ./virtualizer -f Makefile copy-headers

