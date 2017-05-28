BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)
CC:=g++-5

DSVSCA:
	$(CC) $(FLAGS) -c virtualizer.cpp -o $(BUILD)virtualizer.o

debug:
	$(CC) -g $(FLAGS) -c virtualizer.cpp -o $(BUILD)virtualizer.o

copy-headers:
	cp *.h $(INC)
