BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)
CC:=g++-5

DSVSCA:
	$(CC) $(FLAGS) -c wave_util.cpp -o $(BUILD)wave_util.o

debug:
	$(CC) -g $(FLAGS) -c wave_util.cpp -o $(BUILD)wave_util.o

copy-headers:
	cp *.h $(INC)
