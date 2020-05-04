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

int main(int args, char* argv[])
{
	char* inFileName = parseArgs(args, argv);
	
	try
	{
		CrossPuzzle puzzle(inFileName);
	
		puzzle.solve();
		
		#ifdef DEBUG
			std::cout << "\nFinal puzzle:" << std::endl << puzzle;
		#endif
		
		// Make .bmp file
		const char* outFile = getOutFileName(inFileName);
		puzzle.bitmap().write(outFile);
		
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
