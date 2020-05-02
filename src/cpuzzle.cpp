// Author:        Alex Trotta
// Start Date:    5/1/18
// Last Modified: 12/20/18
// File Name:     cpuzzle.cpp

#include <iostream>
#include <fstream>
#include <algorithm>
#include <list>
#include "cpuzzle.h"
#include "bmpMaker.h"

/*---------------------------------------------
A possibilityList contains the length of a fill
and all possible start positions of the fill.
---------------------------------------------*/

struct RoworColumn::possibilityList
{
	unsigned fillLength;
	std::list<unsigned> possiblePositions;
	
	possibilityList() {}
	
	possibilityList(const unsigned fl, const unsigned start, const unsigned end)
		: fillLength(fl)
	{
		for (unsigned i = start; i <= end; i++)
		{
			possiblePositions.push_back(i);
		}
	}
};

bool RoworColumn::isComplete() const
{
	if (complete) return true;
	
	for (unsigned i = 0; i < size; i++)
	{
		if (*grid[i] == -1)
		{
			return false;
		}
	}
	return complete = true;
}

void RoworColumn::printgrid(std::ostream& stream) const
{
	for (unsigned i = 0; i < size; i++)
	{
		if (*(grid[i]) == -1)
			stream << "_ ";
		else
			stream << *(grid[i]) << " ";
	}
	stream << std::endl;
}

std::ostream& operator<<(std::ostream& stream, const RoworColumn& roc)
{
	if(roc.isComplete())
	{
		stream << "Complete. Grid:" << std::endl;
		roc.printgrid(stream);
	}
	else
	{
		stream << "Incomplete. Grid:" << std::endl;
		roc.printgrid(stream);
		
		for (unsigned i = 0; i < roc.fillPosses.size(); i++)
		{
			stream << "Fill #" << i << ", length " << roc.fillPosses[i].fillLength << ": ";
			
			for (auto x : roc.fillPosses[i].possiblePositions)
			{
				stream << x << ' ';
			}
			
			stream << std::endl;
		}
	}
	return (stream << std::endl);
}

/*----------------------------------------------------
Constructs a RoworColumn of length 'siz', referencing
the items in grd, with a list of hints hintList.
----------------------------------------------------*/

RoworColumn::RoworColumn(unsigned siz, int* grd[], const std::list<unsigned>& hintList)
	: size(siz), grid(grd),
	  fillPosses(hintList.size())
{
	#ifdef CPUZZLE_DEBUG
		for (unsigned i : hintList) std::cout << i << ' ';
		std::cout << std::endl;
	#endif
	
	if (fillPosses.empty())
	{
		complete = true;
		for (unsigned i = 0; i < size; i++)
		{
			*(grid[i]) = 0;
		}
	}
	else
	{
		complete = false;
		
		unsigned sum = 0;
		for (unsigned hint : hintList) sum += hint;
		
		unsigned extraSpace = size - (sum + fillPosses.size() - 1);
		
		unsigned minPos = 0;
		unsigned i = 0;
		for (unsigned temp : hintList)
		{
			fillPosses[i] = possibilityList(temp, minPos, minPos + extraSpace);
			minPos += temp + 1;
			i++;
		}
	}
}

/*------------------------------------------
Creates a copy of roc, which references grd.
------------------------------------------*/

RoworColumn::RoworColumn(const RoworColumn& roc, int** grd)
	: size(roc.size), grid(grd), complete(roc.complete),
	  fillPosses(roc.fillPosses.size())
{
	if (!isComplete())
	{
		for (unsigned i = 0; i < fillPosses.size(); i++)
		{
			fillPosses[i] = possibilityList(roc.fillPosses[i]);
		}
	}
}

RoworColumn& RoworColumn::operator=(RoworColumn&& rc)
{
	std::swap(grid,rc.grid);
	std::swap(fillPosses,rc.fillPosses);
	
	size = rc.size;
	complete = rc.complete;
	
	return *this;
}

/*-----------------------------------
Throws a puzzle_error if LL is empty.
-----------------------------------*/

void RoworColumn::throwIfEmpty(const std::list<unsigned>& LL)
{
	if (LL.empty())
		throw CrossPuzzle::puzzle_error("Puzzle is unsolvable");
}

/*------------------------------------------------------
Removes incompatible possibilites from a RoworColumn.
If a fill has no more possibilites, throws puzzle_error,
otherwise returns the number of possibilites removed.
------------------------------------------------------*/

// TODO: Some rules added for the first and last fills can be condensed by 
// making "empty" first and last fills (i.e. cell 0 to -1, length 0 for the
// beginning). Need to double check, but it seems that all of the special
// case rules would collapse into other rules.

unsigned RoworColumn::removeIncompatible()
{
	unsigned changesMade = 0;

	// Check each position for a space that has recently been marked
	for (unsigned i = 0; i < size; i++)
	{
		if (*grid[i] == 0)
		{
			// Remove due to empty spaces
			for (unsigned j = 0; j < fillPosses.size(); j++)
			{
				unsigned length = fillPosses[j].fillLength;
				
				// If the empty space is within the fill,
				// remove the possibility.
				
				// Reference this list, for brevity
				auto& positions = fillPosses[j].possiblePositions;
				for (auto it = positions.begin(); it != positions.end();)
				{
					unsigned start = *it;
					if ((start <= i) && (i < start + length))
					{
						positions.erase(it++);
						throwIfEmpty(positions);
						changesMade++;
					}
					else
					{
						// This seems to segfault if I put this in the loop
						// section, unsure why.
						++it;
					}
				}
			}
		}
		else if (*grid[i] == 1)
		{
			// Remove due to filled spaces
			for (unsigned j = 0; j < fillPosses.size(); j++)
			{
				unsigned length = fillPosses[j].fillLength;
				
				// If the filled space is immediately before or after
				// a possible fill, remove the possibility.
				
				// Reference this list, for brevity
				auto& positions = fillPosses[j].possiblePositions;
				for (auto it = positions.begin(); it != positions.end();)
				{
					unsigned start = *it;
					if (start == i + 1 || start + length == i)
					{
						positions.erase(it++);
						throwIfEmpty(positions);
						changesMade++;
					}
					else
					{
						++it;
					}
				}
			}
			
			// TODO: These two rules would be unneeded if the above
			// change was made	
			
			// Filled space cannot be before the first fill
			while (fillPosses[0].possiblePositions.back() > i)
			{
				fillPosses[0].possiblePositions.pop_back();
				throwIfEmpty(fillPosses[0].possiblePositions);
				changesMade++;
			}
			
			// ... or after the last fill.
			unsigned len = fillPosses.back().fillLength;
			while (fillPosses.back().possiblePositions.front() + len < i)
			{
				fillPosses.back().possiblePositions.pop_front();
				throwIfEmpty(fillPosses.back().possiblePositions);
				changesMade++;
			}
			
			// Filled space cannot fall between two adjacent fills
			for (unsigned j = 1; j < fillPosses.size(); j++)
			{
				// If the first of the two fills cannot reach the filled
				// space, the second cannot start after it.
				
				if (fillPosses[j - 1].possiblePositions.back() + 
					fillPosses[j - 1].fillLength < i)
				{
					// Delete all possibilites that start after the filled
					// space.
					while (i < fillPosses[j].possiblePositions.back())
					{
						fillPosses[j].possiblePositions.pop_back();
						throwIfEmpty(fillPosses[j].possiblePositions);
						changesMade++;
					}
				}
				// If the second cannot reach it, the first cannot end
				// before it.
				if (i < fillPosses[j].possiblePositions.front())
				{
					// Delete all possibilites that end before the filled
					// space.
					while (fillPosses[j - 1].possiblePositions.front()
						+ fillPosses[j - 1].fillLength < i)
					{
						fillPosses[j - 1].possiblePositions.pop_front();
						throwIfEmpty(fillPosses[j - 1].possiblePositions);
						changesMade++;
					}
				}
			}
		}
	}
	
	// Push back/Push forward correcting
	
	// Possible TODO: Combine this with last rule in markConsistent, or the rule above
	for (unsigned i = 1; i < fillPosses.size(); i++)
	{
		unsigned furthestback = fillPosses[i - 1].possiblePositions.front() +
			fillPosses[i - 1].fillLength;
		while (furthestback >= fillPosses[i].possiblePositions.front())
		{
			fillPosses[i].possiblePositions.pop_front();
			throwIfEmpty(fillPosses[i].possiblePositions);
			changesMade++;
		}
	}
	
	for (unsigned i = fillPosses.size() - 1; i > 0; i--)
	{
		unsigned furthestforward = fillPosses[i].possiblePositions.back();
		unsigned length = fillPosses[i - 1].fillLength;
		while (furthestforward <= fillPosses[i - 1].possiblePositions.back() + length)
		{
			fillPosses[i - 1].possiblePositions.pop_back();
			throwIfEmpty(fillPosses[i - 1].possiblePositions);
			changesMade++;
		}
	}
	
	return changesMade;
}

/*--------------------------------------------------------------------
Marks each cell in grid starting at index start and stopping before
index end to 'value', and increments changesMade for each change made.
--------------------------------------------------------------------*/

void RoworColumn::markInRange(unsigned start, unsigned end,
	int value, unsigned& changesMade)
{
	for (unsigned i = start; i < end; i++)
	{
		if (*(grid[i]) == -1)
		{
			*(grid[i]) = value;
			changesMade++;
		}
		else if (*(grid[i]) != value)
		{
			throw CrossPuzzle::puzzle_error("Puzzle is unsolvable");
		}
	}
}

/*-----------------------------------------
Marks items in the grid that are consistent
with all possibilites in a RoworColumn.
-----------------------------------------*/

unsigned int RoworColumn::markConsistent()
{
	unsigned changesMade = 0;
	
	// Mark filled spaces
	for (unsigned i = 0; i < fillPosses.size(); i++)
	{
		// Mark every cell from the last possible start position of the fill
		// to the first possible end position of the fill. This may not mark anything.
		unsigned lastToMark = fillPosses[i].possiblePositions.front() +
			fillPosses[i].fillLength;
		
		markInRange(fillPosses[i].possiblePositions.back(), lastToMark, 1, changesMade);
	}
	
	// Mark empty spaces
	
	// Mark every cell that is before all possible start positions of the first
	// fill as empty.
	
	markInRange(0, fillPosses[0].possiblePositions.front(), 0, changesMade);
	
	
	// Mark every cell that is after all possible end positions of the last
	// fill as empty.
	
	unsigned int lastoflast = fillPosses.back().possiblePositions.back()
		+ fillPosses.back().fillLength;
	markInRange(lastoflast, size, 0, changesMade);
	
	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned int i = 1; i < fillPosses.size(); i++)
	{
		unsigned int lastoffirst = fillPosses[i - 1].possiblePositions.back()
			+ fillPosses[i - 1].fillLength;
		unsigned int firstoflast = fillPosses[i].possiblePositions.front();
		
		markInRange(lastoffirst, firstoflast, 0, changesMade);
	}
	
	return changesMade;
}

/*--------------------------------------------------------------
GetList returns a list containing one line of ints read from in.
--------------------------------------------------------------*/

std::list<unsigned int> CrossPuzzle::getList(std::ifstream& in)
{
	unsigned int temp;
	std::list<unsigned int> L;

	// There will always be at least one number
	in >> temp;
	L.push_front(temp);

	for (int c = in.peek(); c != '\n' && !in.eof(); c = in.peek())
	{
		if ('0' <= c && c <= '9')
		{
			in >> temp;
			L.push_back(temp);
		}
		else
		{
			in.ignore();
		}
	}
	
	// If the list is just 0, let it be an empty list
	if (L.size() == 1 && L.front() == 0)
	{
		L.pop_front();
	}

	return L;
}

std::ostream& operator<<(std::ostream& stream, const CrossPuzzle& CP)
{
	stream << "===========================================================" << std::endl;
	stream << "Rows: " << CP.numrows << ", Columns: " << CP.numcols << std::endl << std::endl;
	
	if (!CP.isComplete())
	{
		for (unsigned int i = 0; i < CP.numrows; i++)
		{
			stream << "Row " << i << ": " << CP.rows[i];
		}
		for (unsigned int i = 0; i < CP.numcols; i++)
		{
			stream << "Column " << i << ": " << CP.cols[i];
		}
	}
	
	for (unsigned int i = 0; i < CP.numrows; i++)
	{
		for (unsigned int j = 0; j < CP.numcols; j++)
		{
			if (CP.grid[i][j] == -1)
			{
				stream << "_ ";
			}
			else
			{
				stream << CP.grid[i][j] << " ";
			}
		}
		stream << std::endl;
	}
	
	stream << "===========================================================" << std::endl;
	return stream;
}

void CrossPuzzle::createGrid()
{
	grid.resize(numrows);
	for (unsigned int i = 0; i < numrows; i++)
	{
		grid[i].resize(numcols);
		for (unsigned int j = 0; j < numcols; j++)
		{
			grid[i][j] = -1;
		}
	}
}

int** CrossPuzzle::createTempGridRow(unsigned int i)
{
	int** tempgrid = new int* [numcols];
	for (unsigned int j = 0; j < numcols; j++)
	{
		tempgrid[j] = &(grid[i][j]);
	}
	return tempgrid;
}

int** CrossPuzzle::createTempGridCol(unsigned int i)
{
	int** tempgrid = new int* [numrows];
	for (unsigned int j = 0; j < numrows; j++)
	{
		tempgrid[j] = &(grid[j][i]);
	}
	return tempgrid;
}

/*--------------------------------------------------------------------
Creates a CrossPuzzle from information in an input file, named infile.
--------------------------------------------------------------------*/

CrossPuzzle::CrossPuzzle(const char* infile)
{
	std::ifstream ifs(infile);
	
	if (!ifs.is_open())
	{
		throw std::exception();
	}
	
	ifs >> numcols;
	ifs >> numrows;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Rows: " << numrows << ", Cols: " << numcols << std::endl;
	#endif
	
	createGrid();
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Row hintlists:" << std::endl;
	#endif
	
	rows = new RoworColumn[numrows];
	for (unsigned int i = 0; i < numrows; i++)
	{
		#ifdef CPUZZLE_DEBUG
			std::cout << "    " << i << ": ";
		#endif
		
		rows[i] = RoworColumn(numcols, createTempGridRow(i), getList(ifs));
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Column hintlists:" << std::endl;
	#endif
	
	cols = new RoworColumn[numcols];
	for (unsigned int i = 0; i < numcols; i++)
	{
		#ifdef CPUZZLE_DEBUG
			std::cout << "    " << i << ": ";
		#endif
		
		cols[i] = RoworColumn(numrows, createTempGridCol(i), getList(ifs));
	}

	ifs.close();

	#ifdef CPUZZLE_DEBUG
		std::cout << "Successfully read from file." << std::endl
			<< std::endl << "Puzzle:" << std::endl << *this;
	#endif
}

/*-------------------
Creates a copy of CP.
-------------------*/

CrossPuzzle::CrossPuzzle(const CrossPuzzle& CP)
	: numrows(CP.numrows), numcols(CP.numcols)
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Copy constructor (CrossPuzzle)" << std::endl;
	#endif
	createGrid();
	
	for (unsigned int i = 0; i < numrows; i++)
	{
		for (unsigned int j = 0; j < numcols; j++)
		{
			grid[i][j] = CP.grid[i][j];
		}
	}
	
	rows = new RoworColumn[numrows];
	for (unsigned int i = 0; i < numrows; i++)
	{
		rows[i] = RoworColumn(CP.rows[i], createTempGridRow(i));
	}
	
	cols = new RoworColumn[numcols];
	for (unsigned int i = 0; i < numcols; i++)
	{
		cols[i] = RoworColumn(CP.cols[i], createTempGridCol(i));
	}
}

CrossPuzzle& CrossPuzzle::operator=(const CrossPuzzle& CP)
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Copy assignment (CrossPuzzle)" << std::endl;
	#endif
	
	if (numrows != CP.numrows)
	{
		delete rows;
		numrows = CP.numrows;
		rows = new RoworColumn[numrows];
	}
	
	if (numcols != CP.numcols)
	{
		delete cols;
		numcols = CP.numcols;
		cols = new RoworColumn[numcols];
	}
	
	createGrid();
	
	for (unsigned int i = 0; i < numrows; i++)
	{
		for (unsigned int j = 0; j < numcols; j++)
		{
			grid[i][j] = CP.grid[i][j];
		}
	}
	
	for (unsigned int i = 0; i < numrows; i++)
	{
		rows[i] = RoworColumn(CP.rows[i], createTempGridRow(i));
	}
	
	for (unsigned int i = 0; i < numcols; i++)
	{
		cols[i] = RoworColumn(CP.cols[i], createTempGridCol(i));
	}
	
	return *this;
}

CrossPuzzle& CrossPuzzle::operator=(CrossPuzzle&& CP)
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Move assignment (CrossPuzzle)" << std::endl;
	#endif
	
	std::swap(numrows,CP.numrows);
	std::swap(rows,CP.rows);
	
	std::swap(numcols,CP.numcols);
	std::swap(cols,CP.cols);
	
	std::swap(grid,CP.grid);
	
	return *this;
}

/*--------------------
Destroys a CrossPuzzle
--------------------*/

CrossPuzzle::~CrossPuzzle()
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Destructor (CrossPuzzle):" << std::endl << *this;
	#endif
	delete[] rows;
	
	delete[] cols;
}

bool CrossPuzzle::isComplete() const
{
	// Only need to check if rows or columns are complete,
	// arbitrarily pick rows.
	for (unsigned int i = 0; i < numrows; i++)
	{
		if (!rows[i].isComplete())
		{
			return false;
		}
	}
	return true;
}

void CrossPuzzle::createBitmap(const char* fileName) const
{
	BMP_24 soln(numrows, numcols);

	for (unsigned int i = 0; i < numrows; i++)
	{
		// The rows in a bitmap are flipped, so when writing,
		// write to the opposite side.
		unsigned int rowNum = numrows - 1 - i;
		for (unsigned int j = 0; j < numcols; j++)
		{
			if (grid[i][j] == 1)
			{
				soln.position(rowNum, j) = color_24::black;
			}
			// base color is white
		}
	}
	soln.write(fileName);
}
