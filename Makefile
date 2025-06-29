LIBS = glfw3 gl
CPPFLAGS = -MD -I inc $(shell pkg-config --cflags $(LIBS))
CFLAGS =
CXXFLAGS = -std=c++23
LDFLAGS = $(shell pkg-config --libs $(LIBS))

CC = gcc-14
CXX = g++-14
LD = g++-14
RM = rm

BIN = ./bin/main

SRC = $(shell find src -type f)
OBJ = $(SRC:src/%=bin/%.o)
DEP = $(OBJ:bin/%.o=bin/%.d)

all:: $(BIN)

$(BIN): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

bin/%.c.o: src/%.c
	$(CC) $(CPPFLAGS) -c -o $@ $< $(CFLAGS)

bin/%.cpp.o: src/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $< $(CXXFLAGS)

run:: $(BIN)
	$(BIN)

clean::
	$(RM) -r bin/*

-include $(DEP)
