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
			
			// If this cell represents row x, column y, then if this line is
			// a row, then this is y, and for a column, x.
			unsigned opposite_line;
			
			// True if a certain set of rules (in removeIncompatible) have
			// been used with this cell.
			bool rules_used;
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
		
		bool needs_line_solving;
		
		line(auto&& grd, const std::vector<unsigned>& hintList, unsigned offset);
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
	void markInRange(std::vector<line::cell>& gridReferences,
		unsigned start, unsigned end, cell_state value);
	
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
