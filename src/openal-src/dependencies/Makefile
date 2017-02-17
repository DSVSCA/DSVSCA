install-deps:
	wget http://openal-soft.org/openal-releases/openal-soft-1.17.2.tar.bz2
	tar -xf openal-soft-1.17.2.tar.bz2
	rm openal-soft-1.17.2.tar.bz2
	cd openal-soft-1.17.2/build; \
	cmake ..; \
	make; \
	make install
	rm -rf openal-soft-1.17.2/
	wget https://github.com/vancegroup/freealut/archive/master.zip
	unzip master.zip
	rm master.zip
	cd freealut-master; \
	mkdir build; \
	cd build; \
	cmake .. -DCMAKE_INSTALL_PREFIX:STRING="/usr/local"; \
	make; \
	make install
	rm -rf freealut-master/
	LD_LIBRARY_PATH=/usr/local/lib
	export LD_LIBRARY_PATH

clean:
	rm -rf openal-soft-1.17.2 openal-soft-1.17.2.tar.bz2 freealut-master master.zip
