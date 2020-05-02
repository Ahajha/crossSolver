// Author:        Alex Trotta
// Start Date:    5/1/18
// Last Modified: 12/20/18
// File Name:     cpuzzle.cpp

#include <iostream>
#include <fstream>
#include <algorithm>
#include "cpuzzle.h"
#include "../../../lib/linkedList.h"
#include "bmpMaker.h"

/*---------------------------------------------
A possibilityList contains the length of a fill
and all possible start positions of the fill.
---------------------------------------------*/

struct RoworColumn::possibilityList
{
	unsigned fillLength;
	LinkedList<unsigned> possiblePositions;
	
	possibilityList() {}
	
	possibilityList(const unsigned fl, const unsigned start, const unsigned end)
		: fillLength(fl)
	{
		for (unsigned i = start; i <= end; i++)
		{
			possiblePositions.insertBack(i);
		}
	}
};

bool RoworColumn::isComplete()
{
	return complete = constIsComplete();
}

bool RoworColumn::constIsComplete() const
{
	if (complete)
	{
		return true;
	}
	
	for (unsigned i = 0; i < size; i++)
	{
		if (*grid[i] == -1)
		{
			return false;
		}
	}
	return true;
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
	if(roc.constIsComplete())
	{
		stream << "Complete. Grid:" << std::endl;
		roc.printgrid(stream);
	}
	else
	{
		stream << "Incomplete. Grid:" << std::endl;
		roc.printgrid(stream);
		
		for (unsigned i = 0; i < roc.numFills; i++)
		{
			stream << "Fill #" << i << ", length " << roc.fillPosses[i].fillLength << ": "
				<< roc.fillPosses[i].possiblePositions << std::endl;
		}
	}
	return (stream << std::endl);
}

/*----------------------------------------------------
Constructs a RoworColumn of length 'siz', referencing
the items in grd, with a list of hints hintList.
----------------------------------------------------*/

RoworColumn::RoworColumn(unsigned siz, int* grd[], const LinkedList<unsigned>& hintList)
	: size(siz), numFills(hintList.getSize()), grid(grd),
	  fillPosses(new possibilityList[numFills])
{
	#ifdef CPUZZLE_DEBUG
		std::cout << hintList << std::endl;
	#endif
	
	if (numFills == 0)
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
		unsigned extraSpace = size - (hintList.sum() + numFills - 1);
		
		unsigned minPos = 0;
		LinkedList<unsigned>::ConstIterator iter = hintList.iterator();
		for (unsigned i = 0; i < numFills; i++)
		{
			unsigned temp = iter.next();
			fillPosses[i] = possibilityList(temp, minPos, minPos + extraSpace);
			minPos += temp + 1;
		}
	}
}

/*------------------------------------------
Creates a copy of roc, which references grd.
------------------------------------------*/

RoworColumn::RoworColumn(const RoworColumn& roc, int** grd)
	: size(roc.size), numFills(roc.numFills), grid(grd), complete(roc.complete),
	  fillPosses(new possibilityList[numFills])
{
	if (!isComplete())
	{
		for (unsigned i = 0; i < numFills; i++)
		{
			fillPosses[i] = possibilityList(roc.fillPosses[i]);
		}
	}
}

RoworColumn::RoworColumn() : fillPosses(NULL) {}

/*--------------------
Destroys a RoworColumn
--------------------*/

RoworColumn::~RoworColumn()
{
	delete[] fillPosses;
}

RoworColumn& RoworColumn::operator=(const RoworColumn& rc)
{
	size = rc.size;
	numFills = rc.numFills;
	grid = rc.grid;
	complete = rc.complete;
	fillPosses = new possibilityList[numFills];
	
	if (!complete)
	{
		for (unsigned i = 0; i < numFills; i++)
		{
			fillPosses[i] = possibilityList(rc.fillPosses[i]);
		}
	}
	
	return *this;
}

RoworColumn& RoworColumn::operator=(RoworColumn&& rc)
{
	std::swap(grid,rc.grid);
	std::swap(fillPosses,rc.fillPosses);
	
	size = rc.size;
	numFills = rc.numFills;
	complete = rc.complete;
	
	return *this;
}

/*-----------------------------------
Throws a puzzle_error if LL is empty.
-----------------------------------*/

void RoworColumn::throwIfEmpty(const LinkedList<unsigned>& LL)
{
	if (LL.isEmpty())
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
			for (unsigned j = 0; j < numFills; j++)
			{
				LinkedList<unsigned>::Iterator posses =
					fillPosses[j].possiblePositions.iterator();
					
				unsigned length = fillPosses[j].fillLength;
				
				// If the empty space is within the fill,
				// remove the possibility.
				while (posses.hasNext())
				{
					unsigned start = posses.next();
					if ((start <= i) && (i < start + length))
					{
						posses.removePrevious();
						throwIfEmpty(fillPosses[j].possiblePositions);
						changesMade++;
					}
				}
			}
		}
		else if (*grid[i] == 1)
		{
			// Remove due to filled spaces
			for (unsigned j = 0; j < numFills; j++)
			{
				LinkedList<unsigned>::Iterator posses =
					fillPosses[j].possiblePositions.iterator();
					
				unsigned length = fillPosses[j].fillLength;
				
				// If the filled space is immediately before or after
				// a possible fill, remove the possibility.
				while (posses.hasNext())
				{
					unsigned start = posses.next();
					if (start == i + 1 || start + length == i)
					{
						posses.removePrevious();
						throwIfEmpty(fillPosses[j].possiblePositions);
						changesMade++;
					}
				}
			}
			
			// TODO: These two rules would be unneeded if the above
			// change was made	
			
			// Filled space cannot be before the first fill
			while (fillPosses[0].possiblePositions.peekBack() > i)
			{
				fillPosses[0].possiblePositions.removeBack();
				throwIfEmpty(fillPosses[0].possiblePositions);
				changesMade++;
			}
			
			// ... or after the last fill.
			unsigned len = fillPosses[numFills - 1].fillLength;
			while (fillPosses[numFills - 1].possiblePositions.peekFront() + len < i)
			{
				fillPosses[numFills - 1].possiblePositions.remove();
				throwIfEmpty(fillPosses[numFills - 1].possiblePositions);
				changesMade++;
			}
			
			// Filled space cannot fall between two adjacent fills
			for (unsigned j = 1; j < numFills; j++)
			{
				// If the first of the two fills cannot reach the filled
				// space, the second cannot start after it.
				
				if (fillPosses[j - 1].possiblePositions.peekBack() + 
					fillPosses[j - 1].fillLength < i)
				{
					// Delete all possibilites that start after the filled
					// space.
					while (i < fillPosses[j].possiblePositions.peekBack())
					{
						fillPosses[j].possiblePositions.removeBack();
						throwIfEmpty(fillPosses[j].possiblePositions);
						changesMade++;
					}
				}
				// If the second cannot reach it, the first cannot end
				// before it.
				if (i < fillPosses[j].possiblePositions.peekFront())
				{
					// Delete all possibilites that end before the filled
					// space.
					while (fillPosses[j - 1].possiblePositions.peekFront()
						+ fillPosses[j - 1].fillLength < i)
					{
						fillPosses[j - 1].possiblePositions.remove();
						throwIfEmpty(fillPosses[j - 1].possiblePositions);
						changesMade++;
					}
				}
			}
		}
	}
	
	// Push back/Push forward correcting
	
	// Possible TODO: Combine this with last rule in markConsistent, or the rule above
	for (unsigned i = 1; i < numFills; i++)
	{
		unsigned furthestback = fillPosses[i - 1].possiblePositions.peekFront() +
			fillPosses[i - 1].fillLength;
		while (furthestback >= fillPosses[i].possiblePositions.peekFront())
		{
			fillPosses[i].possiblePositions.remove();
			throwIfEmpty(fillPosses[i].possiblePositions);
			changesMade++;
		}
	}
	
	for (unsigned i = numFills - 1; i > 0; i--)
	{
		unsigned furthestforward = fillPosses[i].possiblePositions.peekBack();
		unsigned length = fillPosses[i - 1].fillLength;
		while (furthestforward <= fillPosses[i - 1].possiblePositions.peekBack() + length)
		{
			fillPosses[i - 1].possiblePositions.removeBack();
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
	for (unsigned i = 0; i < numFills; i++)
	{
		// Mark every cell from the last possible start position of the fill
		// to the first possible end position of the fill. This may not mark anything.
		unsigned lastToMark = fillPosses[i].possiblePositions.peekFront() +
			fillPosses[i].fillLength;
		
		markInRange(fillPosses[i].possiblePositions.peekBack(), lastToMark, 1, changesMade);
	}
	
	// Mark empty spaces
	
	// Mark every cell that is before all possible start positions of the first
	// fill as empty.
	
	markInRange(0, fillPosses[0].possiblePositions.peekFront(), 0, changesMade);
	
	
	// Mark every cell that is after all possible end positions of the last
	// fill as empty.
	
	unsigned int lastoflast = fillPosses[numFills - 1].possiblePositions.peekBack()
		+ fillPosses[numFills - 1].fillLength;
	markInRange(lastoflast, size, 0, changesMade);
	
	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned int i = 1; i < numFills; i++)
	{
		unsigned int lastoffirst = fillPosses[i - 1].possiblePositions.peekBack()
			+ fillPosses[i - 1].fillLength;
		unsigned int firstoflast = fillPosses[i].possiblePositions.peekFront();
		
		markInRange(lastoffirst, firstoflast, 0, changesMade);
	}
	
	return changesMade;
}

/*--------------------------------------------------------------
GetList returns a list containing one line of ints read from in.
--------------------------------------------------------------*/

LinkedList<unsigned int> CrossPuzzle::getList(std::ifstream& in)
{
	unsigned int temp;
	LinkedList<unsigned int> L;

	// There will always be at least one number
	in >> temp;
	L.insert(temp);

	for (int c = in.peek(); c != '\n' && !in.eof(); c = in.peek())
	{
		if ('0' <= c && c <= '9')
		{
			in >> temp;
			L.insertBack(temp);
		}
		else
		{
			in.ignore();
		}
	}
	
	// If the list is just 0, let it be an empty list
	if (L.getSize() == 1 && L.peekFront() == 0)
	{
		L.remove();
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
	grid = new int* [numrows];
	for (unsigned int i = 0; i < numrows; i++)
	{
		grid[i] = new int[numcols];
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
	
	delete grid;
	
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
	for (unsigned int i = 0; i < numrows; i++)
	{
		delete[] grid[i];
	}
	delete[] grid;
	
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
