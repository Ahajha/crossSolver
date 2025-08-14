#include "nonagram.hpp"

#include "ctpl_stl.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace stdfs = std::filesystem;

void parseArgs(int argc, const char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "usage: " << argv[0] << " infolder outfolder\n";
		exit(1);
	}
}

void solve(const stdfs::path infile, const stdfs::path outfile)
{
	static std::mutex iomut;

	auto start = std::chrono::steady_clock::now();

	nonagram puzzle;
	{
		std::ifstream ifs(infile);

		if (!ifs.is_open())
		{
			std::lock_guard lock(iomut);
			std::cout << infile << " input error\n";
			return;
		}

		ifs >> puzzle;
	}

	auto input_done = std::chrono::steady_clock::now();

	if (!puzzle.solve())
	{
		std::lock_guard lock(iomut);
		std::cout << infile << " failure\n";
		return;
	}

	auto solve_done = std::chrono::steady_clock::now();

	puzzle.bitmap().write(outfile);

	auto output_done = std::chrono::steady_clock::now();

	std::lock_guard lock(iomut);
	std::cout << std::setw(12)
			  << std::chrono::duration<double, std::milli>(input_done - start).count()
			  << std::setw(12)
			  << std::chrono::duration<double, std::ratio<1>>(solve_done - input_done).count()
			  << std::setw(12)
			  << std::chrono::duration<double, std::milli>(output_done - solve_done).count()
			  << std::setw(12)
			  << std::chrono::duration<double, std::ratio<1>>(output_done - start).count() << infile
			  << '\n';
}

int main(int argc, const char* argv[])
{
	parseArgs(argc, argv);
	stdfs::path infolder(argv[1]), outfolder(argv[2]);

	ctpl::thread_pool pool;

	stdfs::create_directories(outfolder);

	std::cout << std::left
			  << "input (ms)  "
				 "solve (s)   "
				 "output (ms) "
				 "total (s)   "
				 "input file\n";
	for (auto& infile : stdfs::directory_iterator(infolder))
	{
		if (infile.is_directory())
			continue;

		auto outfile = outfolder / infile.path().filename().replace_extension(".bmp");

		pool.push(solve, infile.path(), outfile);
	}
}
