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
	};
	
	static std::list<unsigned> getList(std::istream& in);
	
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
	
	void evaluateHintList(std::list<unsigned> hintList,
	#ifndef CPUZZLE_DEBUG
		std::vector<unsigned> references);
	#else
		std::vector<unsigned> references, std::string ID);
	#endif
	
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
	
	CrossPuzzle() {}
	
	friend std::istream& operator>>(std::istream& stream, CrossPuzzle& CP);
	
	void solve();
	
	bool isComplete() const;
	BMP_24 bitmap() const;
};

#endif
