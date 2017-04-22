FFMPEG_EXISTS:=$(shell whereis -b libavcodec.a | grep -q "libavcodec.a" && whereis -b libavdevice.a | grep -q "libavdevice.a" && whereis -b libavfilter.a | grep -q "libavfilter.a" && whereis -b libavformat.a | grep -q "libavformat.a" && whereis -b libavutil.a | grep -q "libavutil.a" && whereis -b libswresample.a | grep -q "libswresample.a" && whereis -b libswscale.a | grep -q "libswscale.a"; echo $$?)

default: libFFTConvolver.a libavcodec.a libavdevice.a libavfilter.a libavformat.a libavutil.a libswresample.a libswscale.a libmysofa.a mpv-deps
	cp libFFTConvolver.a ../build/
	cp FFTConvolver/*.h ../include/
ifeq ($(FFMPEG_EXISTS),1)
	@echo "FFMPEG is not installed so we will use our local build."
	cp libavcodec.a ../build/
	cp libavdevice.a ../build/
	cp libavfilter.a ../build/
	cp libavformat.a ../build/
	cp libavutil.a ../build/
	cp libswresample.a ../build/
	cp libswscale.a ../build/
	if [ ! -d "../include/libavcodec" ]; then mkdir ../include/libavcodec; fi
	cp ffmpeg-2.8.11/libavcodec/*.h ../include/libavcodec/
	if [ ! -d "../include/libavdevice" ]; then mkdir ../include/libavdevice; fi
	cp ffmpeg-2.8.11/libavdevice/*.h ../include/libavdevice/
	if [ ! -d "../include/libavfilter" ]; then mkdir ../include/libavfilter; fi
	cp ffmpeg-2.8.11/libavfilter/*.h ../include/libavfilter/
	if [ ! -d "../include/libavformat" ]; then mkdir ../include/libavformat; fi
	cp ffmpeg-2.8.11/libavformat/*.h ../include/libavformat/
	if [ ! -d "../include/libavutil" ]; then mkdir ../include/libavutil; fi
	cp ffmpeg-2.8.11/libavutil/*.h ../include/libavutil/
	if [ ! -d "../include/libswresample" ]; then mkdir ../include/libswresample; fi
	cp ffmpeg-2.8.11/libswresample/*.h ../include/libswresample/
	if [ ! -d "../include/libswscale" ]; then mkdir ../include/libswscale; fi
	cp ffmpeg-2.8.11/libswscale/*.h ../include/libswscale/
endif
	cp libmysofa.a ../build/
	cp libmysofa-master/src/hrtf/mysofa.h ../include/

mpv-deps:
	if ! dpkg -l | grep libva-dev -c >> /dev/null; then sudo apt-get install libva-dev; fi
	if ! dpkg -l | grep liblzma-dev -c >> /dev/null; then sudo apt-get install liblzma-dev; fi
	if ! dpkg -l | grep libbz2-dev -c >> /dev/null; then sudo apt-get install libbz2-dev; fi

libFFTConvolver.a:
	wget https://github.com/HiFi-LoFi/FFTConvolver/archive/non-uniform.zip
	unzip non-uniform.zip
	rm non-uniform.zip
	mv FFTConvolver-non-uniform FFTConvolver
	cd FFTConvolver;\
		g++ -std=c++11 -fpic -c *.cpp;\
		ar rcs libFFTConvolver.a *.o;\
		mv libFFTConvolver.a ../;\
		cp *.h ../../include/

libavcodec.a libavdevice.a libavfilter.a libavformat.a libavutil.a libswresample.a libswscale.a:
ifeq ($(FFMPEG_EXISTS),1)
	@echo "FFMPEG is not installed so we will build it locally."
	wget https://ffmpeg.org/releases/ffmpeg-2.8.11.tar.xz
	tar -xvf ffmpeg-2.8.11.tar.xz
	rm ffmpeg-2.8.11.tar.xz
	cd ffmpeg-2.8.11; ./configure
	$(MAKE) -C ffmpeg-2.8.11
	cp ffmpeg-2.8.11/libavcodec/libavcodec.a ./
	cp ffmpeg-2.8.11/libavdevice/libavdevice.a ./
	cp ffmpeg-2.8.11/libavfilter/libavfilter.a ./
	cp ffmpeg-2.8.11/libavformat/libavformat.a ./
	cp ffmpeg-2.8.11/libavutil/libavutil.a ./
	cp ffmpeg-2.8.11/libswresample/libswresample.a ./
	cp ffmpeg-2.8.11/libswscale/libswscale.a ./
endif

libmysofa.a:
	wget https://github.com/brettsimons/libmysofa/archive/master.zip
	unzip master.zip
	rm master.zip
	cd libmysofa-master/build;\
		CFLAGS="-fpic" CXXFLAGS="-fpic" cmake -DCMAKE_BUILD_TYPE=Debug ..;\
		make mysofa-static
	cp libmysofa-master/build/src/libmysofa.a ./

clean:
	rm -rf FFTConvolver libFFTConvolver.a
