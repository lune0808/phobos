CPPFLAGS = -MD
CFLAGS =
CXXFLAGS =
LDFLAGS =

CC = gcc
CXX = g++
LD = g++
RM = rm

BIN = ./bin/main

SRC = $(shell find src -type f)
OBJ = $(SRC:src/%=bin/%.o)
DEP = $(OBJ:bin/%.o=bin/%.d)

-include $(DEP)

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

