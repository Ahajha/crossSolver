#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <list>
#include "bmp.hpp"

#ifdef CPUZZLE_DEBUG
#include <string>
#endif

class nonagram
{
	enum class cell_state : int { unknown, filled, empty };
	
	struct line
	{
		// The first in the pair is the index in the host nonagram's grid to
		// this refers, the second is a flag that indicates whether a certain
		// set of rules have been used by this cell.
		std::vector<std::pair<unsigned, bool>> grid;
		
		/*------------------------------------------------------------
		A fill has a length and a set of candidate starting positions.
		------------------------------------------------------------*/
		
		struct fill
		{
			unsigned length;
			std::list<unsigned> candidates;
			
			fill(unsigned fl, unsigned start, unsigned end);
		};
		std::vector<fill> fills;
		
		#ifdef CPUZZLE_DEBUG
		std::string ID;
		#endif
		
		line(const std::vector<unsigned>& grd,
			const std::list<unsigned>& hintList);
	};
	
	// Variables
	unsigned numrows, numcols;
	
	// Layout of indexes:
	// Row 0: 0 -> (numcols-1)
	// Row 1: numcols -> (2*numcols-1)
	// ...
	std::vector<cell_state> grid;
	
	std::vector<line> lines;
	
	// Methods related to input
	static std::list<unsigned> getList(std::istream& in);
	
	std::vector<unsigned> createGridReferenceLine(unsigned size,
		unsigned start, unsigned increment);
	
	void evaluateHintList(const std::list<unsigned>& hintList,
	#ifndef CPUZZLE_DEBUG
		std::vector<unsigned>&& references);
	#else
		std::vector<unsigned>&& references, std::string&& ID);
	#endif
	
	// Debugging methods
	#ifdef CPUZZLE_DEBUG
	void print_line(std::ostream& stream, const line& roc) const;
	friend std::ostream& operator<<(std::ostream& stream, const nonagram& CP);
	friend std::ostream& operator<<(std::ostream& stream, cell_state cs);
	#endif
	
	// Methods related to solving
	static void throwIfEmpty(const std::list<unsigned>& L);
	void markInRange(const std::vector<std::pair<unsigned,bool>>& gridReferences,
		unsigned start, unsigned end, cell_state value, unsigned& changesMade);
	
	bool isComplete(const line& lin) const;
	
	unsigned removeIncompatible(line& lin);
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