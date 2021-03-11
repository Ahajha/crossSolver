#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <optional>
#include <deque>
#include "bmp.hpp"

#ifdef CPUZZLE_DEBUG
#include <string>
#endif

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
		
		/*------------------------------------------------------------
		A fill has a length and a set of candidate starting positions.
		------------------------------------------------------------*/
		
		struct fill
		{
			unsigned length;
			std::deque<unsigned> candidates;
			
			fill(unsigned fl, unsigned start, unsigned end);
		};
		std::vector<fill> fills;
		
		#ifdef CPUZZLE_DEBUG
		std::string ID;
		#endif
		
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
	static std::vector<unsigned> getList(std::istream& in);
	
	void evaluateHintList(const std::vector<unsigned>& hintList,
	#ifndef CPUZZLE_DEBUG
		auto&& references, unsigned idx, unsigned offset);
	#else
		auto&& references, unsigned idx, unsigned offset, std::string&& ID);
	#endif
	
	// Debugging methods
	#ifdef CPUZZLE_DEBUG
	void print_line(std::ostream& stream, const line& roc) const;
	friend std::ostream& operator<<(std::ostream& stream, const nonagram& CP);
	friend std::ostream& operator<<(std::ostream& stream, cell_state cs);
	#endif
	
	// Methods related to solving
	static void throwIfEmpty(const std::deque<unsigned>& L);
	void markInRange(const std::vector<line::cell>& gridReferences,
		unsigned start, unsigned end, cell_state value, unsigned& changesMade);
	
	bool isComplete(const line& lin) const;
	
	void removeIncompatible(line& lin);
	unsigned markConsistent(line& lin);
	
	unsigned removeAndMark();
	
	public:
	
	// Thrown when a puzzle is unsolvable
	struct puzzle_error : std::exception {};
	
	friend std::istream& operator>>(std::istream& stream, nonagram& CP);
	
	void solve();
	
	bool isComplete() const;
	BMP_24 bitmap() const;
};
