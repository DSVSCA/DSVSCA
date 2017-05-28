BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)
CC:=g++-5

DSVSCA:
	$(CC) $(FLAGS) -c encoder.cpp -o $(BUILD)encoder.o 

debug:
	$(CC) -g $(FLAGS) -c encoder.cpp -o $(BUILD)encoder.o

copy-headers:
	cp *.h $(INC)
