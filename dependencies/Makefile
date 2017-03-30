default: libFFTConvolver.a
	cp libFFTConvolver.a ../build/
	cp FFTConvolver/*.h ../include/

libFFTConvolver.a:
	wget https://github.com/HiFi-LoFi/FFTConvolver/archive/non-uniform.zip
	unzip non-uniform.zip
	rm non-uniform.zip
	mv FFTConvolver-non-uniform FFTConvolver
	cd FFTConvolver;\
		g++ -std=c++11 -c *.cpp;\
		ar rcs libFFTConvolver.a *.o;\
		mv libFFTConvolver.a ../;\
		cp *.h ../../include/
	cp libFFTConvolver.a ../build/

clean:
	rm -rf FFTConvolver libFFTConvolver.a
