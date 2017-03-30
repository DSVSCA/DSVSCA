DSVSCA: copy-headers
	g++ virtualizer.cpp -lavformat -lavcodec -lavfilter -lavutil -lpthread -lswresample -lswscale -o ../../build/virtualizer.o

debug: copy-headers
	g++ -g virtualizer.cpp -lavformat -lavcodec -lavfilter -lavutil -lpthread -lswresample -lswscale -o ../../build/virtualizer.o

copy-headers:
	cp *.h ../../include/
