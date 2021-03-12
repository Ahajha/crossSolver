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

void parseArgs(int argc, char* argv[])
{
	if (argc < 2 || 3 < argc)
	{
		std::cerr << "usage: " << argv[0] << " infile [outfile]\n";
		exit(1);
	}
}

// Replaces the file extension, if it exists, with ".bmp". If there is no
// file extension, appends ".bmp". For sake of simplicity, does not work
// if a file has no extension and a folder in the path has a . in its name.
std::string getOutFileName(const std::string& inFileName)
{
	return std::string(inFileName,0,inFileName.find_last_of('.')) + ".bmp";
}

int main(int argc, char* argv[])
{
	parseArgs(argc, argv);
	const auto inFileName = argv[1];
	
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
	}
	catch (nonagram::puzzle_error&)
	{
		std::cerr << "No solution.\n";
		return 1;
	}
	
	// If the output file name is not given, generate one.
	// by appending/replacing
	// the file extention with ".bmp".
	std::string outFileName = (argc == 3) ? argv[2] : getOutFileName(inFileName);
	
	// Make .bmp file
	puzzle.bitmap().write(outFileName);
	
	std::cout << "Solution image written to file \""
		<<  outFileName << "\"." << std::endl;
}
