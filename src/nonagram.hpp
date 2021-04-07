#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <optional>
#include <deque>
#include "bmp.hpp"

class nonagram
{
	enum class cell_state : int { unknown, filled, empty };
	
	struct line
	{
		struct cell
		{
			// The index in the host nonogram's grid to which this refers.
			unsigned ref_index;
		};
		
		std::vector<cell> grid;
		
		// A fill has a length and a set of candidate starting positions.
		struct fill
		{
			unsigned length;
			std::deque<unsigned> candidates;
			
			fill(unsigned fl, unsigned start, unsigned end);
		};
		std::vector<fill> fills;
		
		bool needs_line_solving, is_row;
		
		line(auto&& grd, const std::vector<unsigned>& hintList, bool is_r);
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
	
	void evaluateHintList(const std::vector<unsigned>& hintList,
		auto&& references, unsigned idx, unsigned offset);
	
	// Debugging methods
	#ifdef CPUZZLE_DEBUG
	friend std::ostream& operator<<(std::ostream& stream, const line& lin);
	friend std::ostream& operator<<(std::ostream& stream, const nonagram& CP);
	friend std::ostream& operator<<(std::ostream& stream, cell_state cs);
	#endif
	
	// Methods related to solving
	static void throwIfEmpty(const std::deque<unsigned>& L);
	void markInRange(line& lin,
		unsigned start, unsigned end, cell_state value);
	
	void performSingleCellRules(cell_state value, unsigned lin, unsigned idx);
	
	bool isComplete(const line& lin) const;
	
	void removeIncompatible(line& lin);
	void markConsistent(line& lin);
	
	void line_solve();
	
	public:
	
	// Thrown when a puzzle is unsolvable
	struct puzzle_error : std::exception {};
	
	// Reads in a nonagram puzzle from an input stream.
	friend std::istream& operator>>(std::istream& stream, nonagram& CP);
	
	// Solves the puzzle, or throws a puzzle_error if unsolvable.
	void solve();
	
	// Returns true if the puzzle is solved. If solve() does not throw, this
	// will return true.
	bool isComplete() const;
	
	// Creates a bitmap of the solution to this puzzle.
	// Assumes the puzzle is solved.
	BMP_24 bitmap() const;
};
