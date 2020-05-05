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
#include "bmpMaker.h"

struct RoworColumn
{
	std::vector<int*> grid;
	mutable bool complete;
	
	struct possibilityList;
	std::vector<possibilityList> fillPosses;
	
	void printgrid(std::ostream& stream) const;
	
	bool isComplete() const;
	
	friend std::ostream& operator<<(std::ostream& stream, const RoworColumn& roc);
	
	RoworColumn(std::vector<int*> grd, const std::list<unsigned>& hintList);
	RoworColumn(const RoworColumn& rc, std::vector<int*> grd);
	RoworColumn& operator=(const RoworColumn& rc) = default;
	RoworColumn& operator=(RoworColumn&& rc);
	RoworColumn() = default;
	~RoworColumn() = default;
};

class CrossPuzzle
{
	private:
	
	static std::list<unsigned> getList(std::ifstream& in);
	
	std::vector<int*> createGridReferenceLine(unsigned size,
		unsigned start, unsigned increment);
	
	unsigned numrows, numcols;
	
	// Layout of indexes:
	// Row 0: 0 -> (numcols-1)
	// Row 1: numcols -> (2*numcols-1)
	// ...
	std::vector<int> grid;
	
	RoworColumn* rows; // RoC []
	RoworColumn* cols; // RoC []
	
	static void throwIfEmpty(const std::list<unsigned>& LL);
	void markInRange(std::vector<int*> gridReferences, unsigned start,
		unsigned end, int value, unsigned& changesMade);
	
	unsigned removeIncompatible(RoworColumn& roc);
	unsigned markConsistent(RoworColumn& roc);
	
	unsigned removeAndMark(RoworColumn rocs[], unsigned numrocs);
	
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
	CrossPuzzle(const CrossPuzzle& CP);
	CrossPuzzle& operator=(const CrossPuzzle& CP);
	CrossPuzzle& operator=(CrossPuzzle&& CP);
	~CrossPuzzle();
	
	void solve();
	
	bool isComplete() const;
	BMP_24 bitmap() const;
	
	friend std::ostream& operator<<(std::ostream& stream, const CrossPuzzle& CP);
};

#endif
