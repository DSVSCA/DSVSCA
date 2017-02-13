CC=g++
LDLIBS?=-lopenal
ifeq ($(LIBAUDIO),1)
CFLAGS+=-DLIBAUDIO
LDLIBS+=-laudio
else
LDLIBS+=-lalut
endif
APP:=openal-example
OBJS:= $(APP).o

all: $(APP)

%.o: %.cpp
	$(CC) -std=c++11 $(CFLAGS) -c $< -o $@ $(LDLIBS)

$(APP): $(OBJS)
	$(CC) -std=c++11 $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

clean:
	-rm -f $(OBJS) $(APP)

