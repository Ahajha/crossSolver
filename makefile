CFLAGS = -g -Wall -Wextra -Wshadow -Wuninitialized -O3
CC     = g++
LINK   = g++

CFILES = src/bmpMaker.cpp\
         src/cpuzzle.cpp\
         src/crossSolver.cpp

OFILES = $(patsubst src/%.cpp,obj/%.o,$(CFILES))

$(shell mkdir -p bin obj)

all: bin/crossSolve

bin/crossSolve: $(OFILES)
	$(LINK) -o bin/crossSolve $(OFILES)

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@
      
clean: 
	rm -f $(OFILES) bin/crossSolve

puzzles: bin/crossSolve
	$(foreach puzzle_file, $(wildcard Puzzles/puzzle*.txt),\
		./bin/crossSolve $(puzzle_file); \
	)

clear_puzzles:
	rm Puzzles/*.bmp
