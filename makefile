CFLAGS = --std=c++20 -g -Wall -Wextra -Wshadow -Wuninitialized -Wpedantic -O3
CC     = g++
LINK   = g++

$(shell mkdir -p bin obj)

help:
	@echo "\"make all\" to compile"
	@echo "\"make run file=<filename>\" to compile and run on a given file"
	@echo "\"make debug file=<filename>\" to compile with debugging and run on a given file"
	@echo "\"make puzzles\" to compile and run on all puzzles in \"Puzzles\" folder"
	@echo "\"make clean\" to remove all generated binary files"
	@echo "\"make clear_puzzles\" to remove all solution image files from \"Puzzles\" folder"

all: bin/crossSolve bin/crossSolveDebug

run: bin/crossSolve
	./bin/crossSolve $(file)

time: bin/crossSolve
	time -f "\nreal: %E\nuser: %U" ./bin/crossSolve $(file)

debug: bin/crossSolveDebug
	./bin/crossSolveDebug $(file)

bin/crossSolve: obj/cpuzzle.o obj/bmpMaker.o obj/crossSolver.o
	$(LINK) -o bin/crossSolve $^

bin/crossSolveDebug: obj/cpuzzleDebug.o obj/bmpMaker.o obj/crossSolver.o
	$(LINK) -o bin/crossSolveDebug $^

obj/bmpMaker.o: src/bmpMaker.cpp src/bmpMaker.h
obj/cpuzzle.o: src/cpuzzle.cpp src/cpuzzle.h
obj/crossSolver.o: src/crossSolver.cpp

obj/%.o:
	$(CC) $(CFLAGS) -c $< -o $@

obj/cpuzzleDebug.o: src/cpuzzle.cpp src/cpuzzle.h
	$(CC) $(CFLAGS) -D CPUZZLE_DEBUG -c $< -o $@
      
clean: 
	rm -f bin/* obj/*

puzzles: bin/crossSolve
	$(foreach puzzle_file, $(wildcard Puzzles/puzzle*.txt),\
		./bin/crossSolve $(puzzle_file); \
	)

clear_puzzles:
	rm Puzzles/*.bmp
