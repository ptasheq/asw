CC=g++
LD=g++


CFLAGS=-I/usr/include/openmpi-x86_64 -pthread -m64
LDFLAGS=-pthread -m64 -L/usr/lib64/openmpi/lib -lmpi_cxx -lmpi

SRCS=main.cpp process.cpp utils.cpp
OBJECTS=$(SRCS:.cpp=.o)

TARGET=bin/asw

OBJS=$(addprefix ./obj/, $(OBJECTS))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

./obj/%.o: ./src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f bin/* obj/*
