#pragma once

#include "bmp.hpp"
#include "index_generator.hpp"

#include <deque>
#include <iostream>
#include <optional>
#include <vector>

class nonagram
{
	enum class cell_state : int
	{
		unknown,
		filled,
		empty
	};

	struct line
	{
		const index_generator<> grid;

		// A fill has a length and a set of candidate starting positions.
		struct fill
		{
			unsigned length;
			std::deque<unsigned> candidates;

			fill(unsigned fl, unsigned start, unsigned end);
		};
		std::vector<fill> fills;

		bool needs_line_solving, is_row;

		line(index_generator<>&& refs, const std::vector<unsigned>& hintList, bool is_r);
	};

	// Variables
	unsigned numrows, numcols;

	// Layout of indexes:
	// Row 0: 0 -> (numcols-1)
	// Row 1: numcols -> (2*numcols-1)
	// ...
	std::vector<cell_state> grid;

	// Element is empty if the line has been solved
	std::vector<std::optional<line>> lines;

	// Keeps track of the number of lines left to solve.
	unsigned lines_to_solve;

	// Methods related to input
	static void getList(std::istream& in, std::vector<unsigned>& L);

	void evaluateHintList(index_generator<>&& refs, const std::vector<unsigned>& hintList,
	                      unsigned idx, bool is_r);

// Debugging methods
#ifdef CPUZZLE_DEBUG
	friend std::ostream& operator<<(std::ostream& stream, const line& lin);
	friend std::ostream& operator<<(std::ostream& stream, const nonagram& CP);
	friend std::ostream& operator<<(std::ostream& stream, cell_state cs);
#endif

	// Methods related to solving
	[[nodiscard]] bool markInRange(line& lin, unsigned start, unsigned end, cell_state value);

	[[nodiscard]] bool performSingleCellRules(cell_state value, unsigned lin, unsigned idx);

	[[nodiscard]] bool isComplete(const line& lin) const;

	[[nodiscard]] bool removeIncompatible(line& lin);
	[[nodiscard]] bool markConsistent(line& lin);

	[[nodiscard]] bool line_solve();

  public:
	// Reads in a nonagram puzzle from an input stream.
	friend std::istream& operator>>(std::istream& stream, nonagram& CP);

	// Solves the puzzle, or returns false if unsolvable.
	[[nodiscard]] bool solve();

	// Returns true if the puzzle is solved. If solve() returned true, this
	// will also return true.
	bool isComplete() const;

	// Creates a bitmap of the solution to this puzzle.
	// Assumes the puzzle is solved.
	BMP_24 bitmap() const;
};
