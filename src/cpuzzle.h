// Author:        Alex Trotta
// Start Date:    5/1/18
// Last Modified: 12/20/18
// File Name:     cpuzzle.h

#ifndef CPUZZLE_H
#define CPUZZLE_H

#include <iostream>
#include <exception>
#include <vector>
#include <list>
#include <string>
#include "bmpMaker.h"

class CrossPuzzle
{
	private:
	
	struct RoworColumn
	{
		std::vector<unsigned> grid;
		mutable bool complete;
		
		/*---------------------------------------------
		A possibilityList contains the length of a fill
		and all possible start positions of the fill.
		---------------------------------------------*/
		
		struct possibilityList
		{
			unsigned fillLength;
			std::list<unsigned> possiblePositions;
			
			possibilityList(unsigned fl, unsigned start, unsigned end)
				: fillLength(fl)
			{
				for (unsigned i = start; i <= end; i++)
				{
					possiblePositions.push_back(i);
				}
			}
		};
		std::vector<possibilityList> fillPosses;
		
		#ifdef CPUZZLE_DEBUG
		std::string ID;
		#endif
		
		RoworColumn(std::vector<unsigned> grd,
			const std::list<unsigned>& hintList);
		// For the time being, not having a custom copy constructor is
		// a slight efficiency issue, but that will be resolved soon.
	};
	
	static std::list<unsigned> getList(std::ifstream& in);
	
	std::vector<unsigned> createGridReferenceLine(unsigned size,
		unsigned start, unsigned increment);
	
	unsigned numrows, numcols;
	
	// Layout of indexes:
	// Row 0: 0 -> (numcols-1)
	// Row 1: numcols -> (2*numcols-1)
	// ...
	std::vector<int> grid;
	
	std::list<RoworColumn> lines;
	
	#ifdef CPUZZLE_DEBUG
	void printRoC(std::ostream& stream, const RoworColumn& roc) const;
	friend std::ostream& operator<<(std::ostream& stream, const CrossPuzzle& CP);
	#endif
	
	static void throwIfEmpty(const std::list<unsigned>& LL);
	void markInRange(std::vector<unsigned> gridReferences, unsigned start,
		unsigned end, int value, unsigned& changesMade);
	
	bool isComplete(const RoworColumn& roc) const;
	
	unsigned removeIncompatible(RoworColumn& roc);
	unsigned markConsistent(RoworColumn& roc);
	
	unsigned removeAndMark();
	
	public:
	
	class puzzle_error : public std::exception
	{
		const char* msg;
		
		public:
		
		virtual const char* what() const throw()
		{
			return msg;
		}
		
		puzzle_error(const char* m) : msg(m) {}
	};
	
	CrossPuzzle(const char* infile);
	
	void solve();
	
	bool isComplete() const;
	BMP_24 bitmap() const;
};

#endif
