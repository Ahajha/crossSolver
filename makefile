CFLAGS = -g -Wall -Wextra -Wshadow -Wuninitialized -O3
CC     = g++
LINK   = g++

CFILES = src/bmpMaker.cpp\
         src/cpuzzle.cpp\
         src/crossSolver.cpp

OFILES = $(patsubst src/%.cpp,obj/%.o,$(CFILES))

$(shell mkdir -p bin obj)

help:
	@echo "\"make all\" to compile"
	@echo "\"make run file=<filename>\" to compile and run on a given file"
	@echo "\"make debug file=<filename>\" to compile with debugging and run on a given file"
	@echo "\"make puzzles\" to compile and run on all puzzles in \"Puzzles\" folder"
	@echo "\"make clean\" to remove all generated binary files"
	@echo "\"make clear_puzzles\" to remove all solution image files from \"Puzzles\" folder"

all: bin/crossSolve

run: bin/crossSolve
	./bin/crossSolve $(file)

debug: bin/crossSolveDebug
	./bin/crossSolveDebug $(file)

bin/crossSolve: $(OFILES)
	$(LINK) -o bin/crossSolve $(OFILES)

bin/crossSolveDebug: obj/bmpMaker.o obj/crossSolver.o obj/cpuzzleDebug.o
	$(LINK) -o bin/crossSolveDebug obj/bmpMaker.o obj/crossSolver.o obj/cpuzzleDebug.o

obj/cpuzzleDebug: src/cpuzzle.h src/cpuzzle.cpp
	$(CC) $(CFLAGS) -D CPUZZLE_DEBUG -c src/cpuzzle.cpp -o obj/cpuzzleDebug.o

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
