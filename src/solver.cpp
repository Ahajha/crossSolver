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

#include "nonagram.hpp"

#include <filesystem>
#include <fstream>
#include <string>

void parseArgs(int argc, char* argv[])
{
	if (argc < 2 || 3 < argc)
	{
		std::cerr << "usage: " << argv[0] << " infile [outfile]\n";
		exit(1);
	}
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

	if (!puzzle.solve())
	{
		std::cerr << "No solution.\n";
		return 1;
	}

	// If the output file name is not given, generate one.
	// by appending/replacing
	// the file extention with ".bmp".
	std::string outFileName =
		(argc == 3) ? argv[2] : std::filesystem::path(inFileName).replace_extension(".bmp");

	// Make .bmp file
	puzzle.bitmap().write(outFileName);

	std::cout << "Solution image written to file \"" << outFileName << "\"." << std::endl;
}
