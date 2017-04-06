BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)

DSVSCA:
	g++ $(FLAGS) -c filter.cpp -o $(BUILD)filter.o 
	g++ $(FLAGS) -c format.cpp -o $(BUILD)format.o

debug:
	g++ -g $(FLAGS) -c filter.cpp -o $(BUILD)filter.o
	g++ -g $(FLAGS) -c format.cpp -o $(BUILD)format.o

copy-headers:
	cp *.h $(INC)
