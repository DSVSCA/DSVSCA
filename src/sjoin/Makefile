BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)
CC:=g++-5

DSVSCA:
	$(CC) $(FLAGS) -c sjoin.cpp -o $(BUILD)sjoin.o 

debug:
	$(CC) -g $(FLAGS) -c sjoin.cpp -o $(BUILD)sjoin.o

copy-headers:
	cp *.h $(INC)
