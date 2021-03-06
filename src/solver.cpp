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

#include <string>
#include <fstream>
#include "nonagram.hpp"

std::string parseArgs(int args, char* argv[])
{
	if (args != 2)
	{
		std::cerr << "Input format: " << argv[0] << " inputFileName\n";
		exit(1);
	}
	return argv[1];
}

std::string getOutFileName(const std::string& inFileName)
{
	return std::string(inFileName,0,inFileName.find_last_of('.')) + ".bmp";
}

int main(int args, char* argv[])
{
	std::string inFileName = parseArgs(args, argv);
	
	nonagram puzzle;
	
	std::ifstream ifs(inFileName);
	
	if (!ifs.is_open())
	{
		std::cerr << "Could not open file \"" << inFileName << "\" for reading.\n";
		return 1;
	}
	
	ifs >> puzzle;
	
	ifs.close();
	
	try
	{
		puzzle.solve();
		
		// Make .bmp file
		std::string outFileName = getOutFileName(inFileName);
		puzzle.bitmap().write(outFileName);
		
		std::cout << "Solution image written to file \""
			<<  outFileName << "\"." << std::endl;
	}
	catch (nonagram::puzzle_error&)
	{
		std::cerr << "No solution.\n";
		return 1;
	}
}
