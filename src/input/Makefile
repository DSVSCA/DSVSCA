DSVSCA: copy-headers
	g++ -c filter.cpp -lavformat -lavcodec -lavfilter -lavutil -lpthread -lswresample -lswscale -o ../../build/filter.o

debug: copy-headers
	g++ -g -c filter.cpp -lavformat -lavcodec -lavfilter -lavutil -lpthread -lswresample -lswscale -o ../../build/filter.o

copy-headers:
	cp *.h ../../include/
