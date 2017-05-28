LIBS:=-lavfilter -pthread -lswscale -lavformat -lavcodec -lswresample -ldl -lm -lavutil -lFFTConvolver -lmysofa -lz

default:
	ar rcs libDSVSCA.a *.o
	g++ -L./ -shared -o libDSVSCA.so *.o -Wl,--whole-archive -lFFTConvolver -lmysofa -Wl,--no-whole-archive -llzma -lbz2 -lva

darwin:
	ar rcs libDSVSCA.a *.o
	g++  -L./ -shared -o libDSVSCA.so *.o -Wl,-all_load -lFFTConvolver -lmysofa -Wl 

ui:
	jar -cfve DSVSCA.jar DSVSCA *.class *.fxml ./dsvsca/*
