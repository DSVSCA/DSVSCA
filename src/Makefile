default:
	$(MAKE) -C ./sjoin
	$(MAKE) -C ./input
	$(MAKE) -C ./virtualizer
	$(MAKE) -C ./encoder
	g++ -std=c++11 -c main.cpp -I../include/ -o ../build/main.o

debug:
	$(MAKE) -C ./sjoin -f Makefile debug
	$(MAKE) -C ./input -f Makefile debug
	$(MAKE) -C ./virtualizer -f Makefile debug
	g++ -g -std=c++11 -c main.cpp -I../include/ -o ../build/main.o
