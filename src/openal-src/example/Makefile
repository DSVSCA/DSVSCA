CC=g++
LDLIBS?=-lopenal -lalut
APP:=openal-example
OBJS:= $(APP).o
OS := $(shell uname)
ifeq ($(OS), Darwin)
BFLAGS:=-D__APPLE__
endif

all: $(APP)

%.o: %.cpp
	$(CC) -std=c++11 $(CFLAGS) $(BFLAGS) -c $< -o $@ $(LDLIBS)

$(APP): $(OBJS)
	$(CC) -std=c++11 $(CFLAGS) $(BFLAGS) $(OBJS) -o $@ $(LDLIBS)

clean:
	-rm -f $(OBJS) $(APP)

