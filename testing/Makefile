default:
	g++ main.cpp -g -I../include -L../build -std=c++11 -lFFTConvolver -lmysofa -lz -o main

run: default
	./main x1.wav subject_119.sofa

gdb: default
	gdb -tui --args ./main x1.wav subject_119.sofa

clean:
	rm -f main out.*
