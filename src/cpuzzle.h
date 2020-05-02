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

//#define CPUZZLE_DEBUG

class RoworColumn
{
	private:
	
	unsigned int size;
	std::vector<int*> grid;
	mutable bool complete;
	
	struct possibilityList;
	std::vector<possibilityList> fillPosses;
	
	void markInRange(unsigned int start, unsigned int end,
		int value, unsigned int& changesMade);
	
	static void throwIfEmpty(const std::list<unsigned int>& LL);
	void printgrid(std::ostream& stream) const;
	
	public:
	
	bool isComplete() const;
	
	friend std::ostream& operator<<(std::ostream& stream, const RoworColumn& roc);
	
	RoworColumn(unsigned int siz, std::vector<int*> grd,
		const std::list<unsigned int>& hintList);
	RoworColumn(const RoworColumn& rc, std::vector<int*> grd);
	RoworColumn& operator=(const RoworColumn& rc) = default;
	RoworColumn& operator=(RoworColumn&& rc);
	RoworColumn() = default;
	~RoworColumn() = default;
	
	unsigned int removeIncompatible();
	unsigned int markConsistent();
};

class CrossPuzzle
{
	private:
	
	static std::list<unsigned int> getList(std::ifstream& in);
	
	void createGrid();
	
	std::vector<int*> createTempGridRow(unsigned int i);
	std::vector<int*> createTempGridCol(unsigned int i);
	
	public:
	
	unsigned int numrows, numcols;
	
	std::vector<std::vector<int>> grid;
	
	RoworColumn* rows; // RoC []
	RoworColumn* cols; // RoC []
	
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
	
	unsigned removeAndMark(RoworColumn rocs[], unsigned numrocs);
	
	void solve();
	
	bool isComplete() const;
	void createBitmap(const char* fileName) const;
	
	friend std::ostream& operator<<(std::ostream& stream, const CrossPuzzle& CP);
};

#endif
