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

class RoworColumn
{
	private:
	
	std::vector<int*> grid;
	mutable bool complete;
	
	struct possibilityList;
	std::vector<possibilityList> fillPosses;
	
	void markInRange(unsigned start, unsigned end, int value, unsigned& changesMade);
	
	static void throwIfEmpty(const std::list<unsigned>& LL);
	void printgrid(std::ostream& stream) const;
	
	public:
	
	bool isComplete() const;
	
	friend std::ostream& operator<<(std::ostream& stream, const RoworColumn& roc);
	
	RoworColumn(std::vector<int*> grd, const std::list<unsigned>& hintList);
	RoworColumn(const RoworColumn& rc, std::vector<int*> grd);
	RoworColumn& operator=(const RoworColumn& rc) = default;
	RoworColumn& operator=(RoworColumn&& rc);
	RoworColumn() = default;
	~RoworColumn() = default;
	
	unsigned removeIncompatible();
	unsigned markConsistent();
};

class CrossPuzzle
{
	private:
	
	static std::list<unsigned> getList(std::ifstream& in);
	
	std::vector<int*> createTempGridRow(unsigned i);
	std::vector<int*> createTempGridCol(unsigned i);
	
	unsigned numrows, numcols;
	
	// Layout of indexes:
	// Row 0: 0 -> (numcols-1)
	// Row 1: numcols -> (2*numcols-1)
	// ...
	std::vector<int> grid;
	
	RoworColumn* rows; // RoC []
	RoworColumn* cols; // RoC []
	
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
	void createBitmap(const char* fileName) const;
	
	friend std::ostream& operator<<(std::ostream& stream, const CrossPuzzle& CP);
};

#endif
