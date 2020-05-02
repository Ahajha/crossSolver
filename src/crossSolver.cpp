// Author:        Alex Trotta
// Start Date:    4/30/18
// Last Modified: 12/19/18
// File Name:     crossSolver.cpp

/*
Format for the input file:

#columns #rows

row 1 clues (left to right)
row 2 clues
.
.
.
row n clues

column 1 clues (top to bottom)
column 2 clues
.
.
.
column k clues
*/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include "../../../lib/linkedList.h"
#include "cpuzzle.h"

char* parseArgs(int args, char* argv[])
{
	if (args != 2)
	{
		std::cout << "Input format: " << argv[0] << " inputFileName" << std::endl;
		exit(1);
	}
	return argv[1];
}

char* getOutFileName(const char* inFileName)
{
	int len = strlen(inFileName);
	char* out = strcpy(new char[len + 1], inFileName);
	out[len - 3] = 'b';
	out[len - 2] = 'm';
	out[len - 1] = 'p';
	return out;
}

/*-------------------------------------------------------------
Calls removeIncompatible() and markConsistent(), in that order,
on each RoworColumn in rocs. Returns number of changes made, or
throws puzzle_error if any RoworColumn in rocs is unsolvable.
-------------------------------------------------------------*/

unsigned removeAndMark(RoworColumn rocs[], unsigned numrocs)
{
	unsigned changesMade = 0;
	for (unsigned i = 0; i < numrocs; i++)
	{
		if (!rocs[i].isComplete())
		{
			unsigned numremoved = rocs[i].removeIncompatible();
			unsigned nummarked  = rocs[i].markConsistent();
			
			#ifdef CPUZZLE_DEBUG
				if (numremoved || nummarked)
				{
					std::cout << i << ":" << std::endl;
					if (numremoved)
					{
						std::cout << "Removed " << numremoved << " possibilities." << std::endl;
					}
					if (nummarked)
					{
						std::cout << "Marked " << nummarked << " cells." << std::endl;
					}
					std::cout << rocs[i];
				}
			#endif
			
			changesMade += (numremoved + nummarked);
		}
	}

	return changesMade;
}

//CrossPuzzle* solve(CrossPuzzle* CP);

/*
Performs a brute-force solve on CP
*/
/*
CrossPuzzle* bruteForceSolve(CrossPuzzle* CP)
{

}
*/
/*------------------------------------------------------
Solve returns a solved puzzle based on CP, or throws a
CrossPuzzle::puzzle_error if CP could not be solved.
No guarantee if the returned puzzle is CP itself or not.
------------------------------------------------------*/

CrossPuzzle solve(CrossPuzzle& CP)
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Entering solve:" << std::endl << "Puzzle:" << std::endl << CP;
	#endif
	
	int rowChanges, colChanges;
	
	do
	{
		#ifdef CPUZZLE_DEBUG
			std::cout << "Remove and mark on rows:" << std::endl;
		#endif
		rowChanges = removeAndMark(CP.rows, CP.numrows);
		
		#ifdef CPUZZLE_DEBUG
			std::cout << "Remove and mark on columns:" << std::endl;
		#endif
		colChanges = removeAndMark(CP.cols, CP.numcols);
	}
	while(!CP.isComplete() && (rowChanges || colChanges));
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Done with logical rules:" << std::endl;	
	#endif
	
	if (CP.isComplete())
	{
		#ifdef CPUZZLE_DEBUG
			std::cout << "Puzzle complete:" << std::endl << CP;	
		#endif
		
		return CP;
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "\nUsing brute force:\n" << std::endl;
	#endif
	
	// find position to brute force
	unsigned x = 0, y = 0;
	while (CP.grid[x][y] != -1)
	{
		y++;
		if (y >= CP.numcols)
		{
			y = 0;
			x++;
		}
		// Not needed, but just to be sure for now:
		if (x >= CP.numrows)
		{
			std::cout << "Complete puzzle is showing incomplete. Error." << std::endl;
			exit(1);
		}
	}
	
	// Guess 1 first, is significantly quicker on some puzzles.
	for (int guess = 1; guess >= 0; guess--)
	{
		CrossPuzzle copy(CP);
		
		#ifdef CPUZZLE_DEBUG
			std::cout << "Guessing " << guess << " at position ("
				<< x << "," << y << ")" << std::endl;
		#endif
	
		copy.grid[x][y] = guess;
		
		try
		{
			return solve(copy);
		}
		catch (CrossPuzzle::puzzle_error& e) {}
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "No solution." << std::endl;
	#endif
	
	throw CrossPuzzle::puzzle_error("Unsolvable puzzle");
}

int main(int args, char* argv[])
{
	char* inFileName = parseArgs(args, argv);
	
	try
	{
		CrossPuzzle puzzle(inFileName);
	
		puzzle = solve(puzzle);
		
		#ifdef DEBUG
			std::cout << "\nFinal puzzle:" << std::endl << puzzle;
		#endif
		
		// Make .bmp file
		const char* outFile = getOutFileName(inFileName);
		puzzle.createBitmap(outFile);
		std::cout << "Solution image written to file \""
			<<  outFile << "\"." << std::endl;
	}
	catch (CrossPuzzle::puzzle_error& e)
	{
		std::cout << "No solution." << std::endl;
		return 1;
	}
	catch (std::exception& e)
	{
		std::cout << "Could not open file \"" << inFileName << "\" for reading." << std::endl;
		return 1;
	}
}
