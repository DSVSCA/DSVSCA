LIBS:=-l:libDSVSCA.a -lavfilter -pthread -lswscale -lavformat -lavcodec -lswresample -ldl -lm -lavutil -lFFTConvolver -lmysofa -lz  -llzma -lbz2 -lva
FLAGS:=-std=c++11 -I../include/ -L../build/ $(LIBS)

default:
	g++ main.cpp $(FLAGS) -o DSVSCA

debug:
	g++ main.cpp -g $(FLAGS) -o DSVSCA

ui:
	cp ../build/DSVSCA.jar ./
