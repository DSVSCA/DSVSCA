default:
	g++ main.cpp -g -I../include -L../build -std=c++11 -lFFTConvolver -lmysofa -lz -o main

run: default
	./main x1.wav subject_131.sofa

clean:
	rm -f main out.*
