LIBS:=-lavfilter -pthread -lswscale -lavformat -lavcodec -lswresample -ldl -lm -lavutil -lFFTConvolver -lmysofa -lz
CC := g++-5

default:
	ar rcs libDSVSCA.a *.o
	$(CC) -L./ -shared -o libDSVSCA.so *.o -Wl,--whole-archive -lFFTConvolver -lmysofa -Wl,--no-whole-archive -llzma -lbz2 -lva

darwin:
	ar rcs libDSVSCA.a *.o
	$(CC) -L./ -shared -o libDSVSCA.so *.o -all_load -lFFTConvolver -lmysofa 

ui:
	jar -cfve DSVSCA.jar DSVSCA *.class *.fxml ./dsvsca/*
