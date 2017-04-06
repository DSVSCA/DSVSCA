BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)

DSVSCA:
	g++ $(FLAGS) -c virtualizer.cpp -o $(BUILD)virtualizer.o

debug:
	g++ -g $(FLAGS) -c virtualizer.cpp -o $(BUILD)virtualizer.o

copy-headers:
	cp *.h $(INC)
