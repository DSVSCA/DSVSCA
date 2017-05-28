BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)
CC:=g++-5

DSVSCA:
	$(CC) $(FLAGS) -c filter.cpp -o $(BUILD)filter.o 
	$(CC) $(FLAGS) -c format.cpp -o $(BUILD)format.o

debug:
	$(CC) -g $(FLAGS) -c filter.cpp -o $(BUILD)filter.o
	$(CC) -g $(FLAGS) -c format.cpp -o $(BUILD)format.o

copy-headers:
	cp *.h $(INC)
