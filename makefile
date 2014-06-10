CC=mpic++
LD=mpic++

CFLAGS=
LDFLAGS=

SRCS=main.cpp process.cpp utils.cpp
OBJECTS=$(SRCS:.cpp=.o)

TARGET=bin/asw


OBJS=$(addprefix ./obj/, $(OBJECTS))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

./obj/%.o: ./src/%.cpp
	@mkdir -p bin log obj
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f bin/* obj/*
