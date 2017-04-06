BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)

DSVSCA:
	g++ $(FLAGS) -c sjoin.cpp -o $(BUILD)sjoin.o 

debug:
	g++ -g $(FLAGS) -c sjoin.cpp -o $(BUILD)sjoin.o

copy-headers:
	cp *.h $(INC)
