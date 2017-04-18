BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)

DSVSCA:
	g++ $(FLAGS) -c wave_util.cpp -o $(BUILD)wave_util.o

debug:
	g++ -g $(FLAGS) -c wave_util.cpp -o $(BUILD)wave_util.o

copy-headers:
	cp *.h $(INC)
