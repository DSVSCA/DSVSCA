DSVSCA:
	g++ "../main.cpp" filter.cpp format.cpp -lavformat -lavcodec -lavfilter -lavutil -lpthread -lswresample -lswscale -o DSVSCA

debug:
	g++ -g "../main.cpp" filter.cpp format.cpp -lavformat -lavcodec -lavfilter -lavutil -lpthread -lswresample -lswscale -o DSVSCA_debug

clean:
	rm -f DSVSCA_debug DSVSCA
