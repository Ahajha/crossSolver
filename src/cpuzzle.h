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

class CrossPuzzle;

struct RoworColumn
{
	std::vector<unsigned> grid;
	mutable bool complete;
	
	struct possibilityList;
	std::vector<possibilityList> fillPosses;
	
	//void printgrid(std::ostream& stream) const;
	
	//friend std::ostream& operator<<(std::ostream& stream, const RoworColumn& roc);
	
	RoworColumn(CrossPuzzle& CP, std::vector<unsigned> grd,
		const std::list<unsigned>& hintList);
	// For the time being, not having a custom copy constructor is
	// a slight efficiency issue, but that will be resolved soon.
	RoworColumn() = default;
};

class CrossPuzzle
{
	private:
	
	friend class RoworColumn;
	
	static std::list<unsigned> getList(std::ifstream& in);
	
	std::vector<unsigned> createGridReferenceLine(unsigned size,
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
	void markInRange(std::vector<unsigned> gridReferences, unsigned start,
		unsigned end, int value, unsigned& changesMade);
	
	bool isComplete(const RoworColumn& roc) const;
	
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
