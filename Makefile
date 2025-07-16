DIR = $(shell find inc -type d)
BINDIR = $(DIR:inc%=bin%)
LIBS = glfw3 gl
CPPFLAGS = -MMD -fpch-deps -Ibin -Iinc $(shell pkg-config --cflags $(LIBS)) -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_XYZW_ONLY
CFLAGS =
CXXFLAGS = -std=c++23 -ggdb3
LDFLAGS = $(shell pkg-config --libs $(LIBS))

CC = gcc-14
CXX = g++-14
LD = g++-14
RM = rm

BIN = ./bin/main

HDR = $(shell find inc -type f)
GCH = $(HDR:inc/%=bin/%.gch)

SRC = $(shell find src -type f)
OBJ = $(SRC:src/%=bin/%.o)

DEP = $(GCH:bin/%.gch=bin/%.d) $(OBJ:bin/%.o=bin/%.d)

all:: $(BINDIR) $(GCH) $(BIN)

$(BIN): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

bin/%.c.o: src/%.c
	$(CC) $(CPPFLAGS) -c -o $@ $< $(CFLAGS)

bin/%.cpp.o: src/%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $< $(CXXFLAGS)

bin/%.h.gch: inc/%.h
	$(CC) $(CPPFLAGS) -o $@ $< $(CFLAGS)

bin/%.hpp.gch: inc/%.hpp
	$(CC) $(CPPFLAGS) -o $@ $< $(CXXFLAGS)

run:: all
	$(BIN)

clean::
	$(RM) -rf bin

$(BINDIR): %:
	mkdir -p $@

-include $(DEP)
