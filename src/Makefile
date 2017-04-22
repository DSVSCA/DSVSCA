BUILD:=../build/
INC:=../include/
FLAGS:=-std=c++11 -fpic -I$(INC)

default: copy-headers
	$(MAKE) -C ./sjoin
	$(MAKE) -C ./input
	$(MAKE) -C ./virtualizer
	$(MAKE) -C ./encoder
	$(MAKE) -C ./wave_util
	g++ $(FLAGS) -c DSVSCA.cpp -o $(BUILD)DSVSCA.o

debug: copy-headers
	$(MAKE) -C ./sjoin -f Makefile debug
	$(MAKE) -C ./input -f Makefile debug
	$(MAKE) -C ./virtualizer -f Makefile debug
	$(MAKE) -C ./encoder -f Makefile debug
	$(MAKE) -C ./wave_util -f Makefile debug
	g++ -g $(FLAGS) -c DSVSCA.cpp -o $(BUILD)DSVSCA.o

ui-default:
	$(MAKE) -C ./ui

ui-debug:
	$(MAKE) -C ./ui -f Makefile debug

copy-headers:
	$(MAKE) -C ./sjoin -f Makefile copy-headers
	$(MAKE) -C ./input -f Makefile copy-headers
	$(MAKE) -C ./virtualizer -f Makefile copy-headers
	$(MAKE) -C ./encoder -f Makefile copy-headers
	$(MAKE) -C ./wave_util -f Makefile copy-headers
	cp *.h $(INC)

