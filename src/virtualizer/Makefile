DSVSCA: copy-headers
	g++ -std=c++11 -I../../include -c virtualizer.cpp -o ../../build/virtualizer.o

debug: copy-headers
	g++ -std=c++11 -g -I../../include -c virtualizer.cpp -o ../../build/virtualizer.o

copy-headers:
	cp *.h ../../include/
