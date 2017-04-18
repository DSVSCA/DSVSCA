LIBS:=-l:libDSVSCA.a -lavfilter -pthread -lswscale -lavformat -lavcodec -lswresample -ldl -lm -lavutil -lFFTConvolver -lmysofa -lz  -llzma -lbz2
FLAGS:=-std=c++11 -I../include/ -L../build/ $(LIBS)

default:
	g++ main.cpp $(FLAGS) -o DSVSCA
	cp ../build/DSVSCA.jar ./

debug:
	g++ main.cpp -g $(FLAGS) -o DSVSCA
	cp ../build/DSVSCA.jar ./
