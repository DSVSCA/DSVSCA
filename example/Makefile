LIBS:=-lDSVSCA -lavfilter -pthread -lswscale -lavformat -lavcodec -lswresample -ldl -lm -lavutil -lFFTConvolver -lmysofa -lz
FLAGS:=-std=c++11 -I../include/ -L../build/ $(LIBS)

default:
	g++ main.cpp $(FLAGS) -o DSVSCA
