
DSVSCA: copy-headers
	g++ -I../../include/ -c filter.cpp -o ../../build/filter.o 
	g++ -I../../include/ -c format.cpp -o ../../build/format.o

debug: copy-headers
	g++ -g -I../../include/ -c filter.cpp -o ../../build/filter.o
	g++ -g -I../../include/ -c format.cpp -o ../../build/format.o

copy-headers:
	cp *.h ../../include/
