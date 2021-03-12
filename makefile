CFLAGS = -std=c++20 -g -Wall -Wextra -Wshadow -Wuninitialized -Wpedantic -O3
CC     = g++
LINK   = g++

$(shell mkdir -p bin obj)

help:
	@echo "\"make all\" to compile all executables"
	@echo "\"make run in=<filename> [out=<filename>]\" to compile and run on a"\
		"given file, optionally specifying the output file"
	@echo "\"make debug in=<filename> [out=<filename>]\" to compile with debugging"\
		"and run on a given file, optionally specifying the output file"
	@echo "\"make batch in=<foldername> [out=<foldername>]\" to compile and run"\
		"on all files in a given folder, optionally specifying the folder to"\
		"place the solution files"
	@echo "\"make puzzles\" to compile and run on all puzzles in \"Puzzles\" folder"
	@echo "\"make clean\" to remove all generated binary files"
	@echo "\"make clear_puzzles\" to remove all solution image files from \"Puzzles\" folder"

all: bin/solve bin/solveDebug

run: bin/solve
	./bin/solve $(in) $(out)

batch: bin/batch_solve
	./bin/batch_solve $(in) $(out)

time: bin/solve
	time -f "\nreal: %E\nuser: %U" ./bin/solve $(in) $(out)

debug: bin/solveDebug
	./bin/solveDebug $(in) $(out)

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
