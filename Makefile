
CXX = clang++
CXXFLAGS = -std=c++11

OBJ = lex.o parse.o obj.o main.o
HDR = parse.h obj.h que.h lex.h
BIN = infix

# standard build
all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ)

$(OBJ): $(HDR)

clean:
	rm $(OBJ) $(BIN)

# debug option
debug: CXXFLAGS += -DDEBUG -g
debug: all
