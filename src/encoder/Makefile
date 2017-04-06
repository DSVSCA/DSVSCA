BUILD:=../../build/
INC:=../../include/
FLAGS:=-std=c++11 -fpic -I$(INC)

DSVSCA:
	g++ $(FLAGS) -c encoder.cpp -o $(BUILD)encoder.o 

debug:
	g++ -g $(FLAGS) -c encoder.cpp -o $(BUILD)encoder.o

copy-headers:
	cp *.h $(INC)
