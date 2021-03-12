CFLAGS = -std=c++20 -g -Wall -Wextra -Wshadow -Wuninitialized -Wpedantic -O3
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

all: bin/solve bin/solveDebug

run: bin/solve
	./bin/solve $(file)

batch: bin/batch_solve
	./bin/batch_solve $(file)

time: bin/solve
	time -f "\nreal: %E\nuser: %U" ./bin/solve $(file)

debug: bin/solveDebug
	./bin/solveDebug $(file)

bin/solve: obj/nonagram.o obj/bmp.o obj/solver.o
bin/solveDebug: obj/nonagramDebug.o obj/bmp.o obj/solver.o

bin/%:
	$(LINK) -o $@ $^
	
bin/batch_solve: obj/nonagram.o obj/bmp.o obj/batch_solver.o
	$(LINK) -pthread -o $@ $^

obj/bmp.o: src/bmp.cpp src/bmp.hpp
obj/nonagram.o: src/nonagram.cpp src/nonagram.hpp
obj/solver.o: src/solver.cpp src/bmp.hpp src/nonagram.hpp
obj/batch_solver.o: src/batch_solver.cpp src/bmp.hpp src/nonagram.hpp

obj/%.o:
	$(CC) $(CFLAGS) -c $< -o $@

obj/nonagramDebug.o: src/nonagram.cpp src/nonagram.hpp
	$(CC) $(CFLAGS) -D CPUZZLE_DEBUG -c $< -o $@
      
clean: 
	rm -f bin/* obj/*

puzzles: bin/solve
	$(foreach puzzle_file, $(wildcard Puzzles/puzzle*.txt),\
		./bin/solve $(puzzle_file);\
	)

clear_puzzles:
	rm Puzzles/*.bmp
