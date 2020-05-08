// Author:        Alex Trotta
// Start Date:    5/1/18
// Last Modified: 12/20/18
// File Name:     cpuzzle.cpp

#include <iostream>
#include <list>
#include "cpuzzle.h"
#include "bmpMaker.h"

/*----------------------------------------------------
Constructs a RoworColumn of length 'siz', referencing
the items in grd, with a list of hints hintList.
----------------------------------------------------*/

CrossPuzzle::RoworColumn::RoworColumn(std::vector<unsigned> grd,
	const std::list<unsigned>& hintList) : grid(grd)
{
	fillPosses.reserve(hintList.size());
	
	unsigned sum = 0;
	for (unsigned hint : hintList) sum += hint;
	
	unsigned extraSpace = grid.size() - (sum + hintList.size() - 1);
	
	unsigned minPos = 0;
	for (unsigned hint : hintList)
	{
		fillPosses.emplace_back(hint, minPos, minPos + extraSpace);
		minPos += hint + 1;
	}
}

/*--------------------------------------------------------------
GetList returns a list containing one line of ints read from in.
--------------------------------------------------------------*/

std::list<unsigned> CrossPuzzle::getList(std::istream& in)
{
	unsigned temp;
	std::list<unsigned> L;
	
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

std::vector<unsigned> CrossPuzzle::createGridReferenceLine(unsigned size,
	unsigned start, unsigned increment)
{
	std::vector<unsigned> tempgrid(size);
	
	unsigned pos = start;
	for (unsigned j = 0; j < size; j++)
	{
		tempgrid[j] = pos;
		pos += increment;
	}
	return tempgrid;
}

void CrossPuzzle::evaluateHintList(std::list<unsigned> hintList,
#ifndef CPUZZLE_DEBUG
	std::vector<unsigned> references)
#else
	std::vector<unsigned> references, std::string ID)
#endif
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "    " << ID << ": ";
		for (unsigned hint : hintList) std::cout << hint << ' ';
		std::cout << std::endl;
	#endif
	
	if (hintList.empty())
	{
		for (unsigned ref : references)
		{
			grid[ref] = 0;
		}
	}
	else
	{
		lines.emplace_back(references, hintList);
		#ifdef CPUZZLE_DEBUG
		lines.back().ID = ID;
		#endif
	}
}

#ifdef CPUZZLE_DEBUG
void CrossPuzzle::printRoC(std::ostream& stream, const RoworColumn& roc) const
{
	if(isComplete(roc))
	{
		stream << "Complete. Grid:" << std::endl;
	}
	else
	{
		stream << "Incomplete. Grid:" << std::endl;
	}
	
	for (unsigned i = 0; i < roc.grid.size(); i++)
	{
		if (grid[roc.grid[i]] == -1)
			stream << "_ ";
		else
			stream << grid[roc.grid[i]] << " ";
	}
	stream << std::endl;
	
	if (!isComplete(roc))
	{
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
	stream << std::endl;
}

std::ostream& operator<<(std::ostream& stream, const CrossPuzzle& CP)
{
	stream << "===========================================================" << std::endl;
	stream << "Rows: " << CP.numrows << ", Columns: " << CP.numcols << std::endl << std::endl;
	
	// If CP is complete, this will print nothing
	for (auto& roc : CP.lines)
	{
		stream << roc.ID << ": ";
		CP.printRoC(stream,roc);
	}
	
	unsigned pos = 0;
	for (unsigned i = 0; i < CP.numrows; i++)
	{
		for (unsigned j = 0; j < CP.numcols; j++)
		{
			if (CP.grid[pos] == -1)
			{
				stream << "_ ";
			}
			else
			{
				stream << CP.grid[pos] << " ";
			}
			++pos;
		}
		stream << std::endl;
	}
	
	stream << "===========================================================" << std::endl;
	return stream;
}
#endif

/*-----------------------------------
Throws a puzzle_error if LL is empty.
-----------------------------------*/

void CrossPuzzle::throwIfEmpty(const std::list<unsigned>& LL)
{
	if (LL.empty())
		throw CrossPuzzle::puzzle_error();
}

/*--------------------------------------------------------------------
Marks each cell in grid starting at index start and stopping before
index end to 'value', and increments changesMade for each change made.
--------------------------------------------------------------------*/

void CrossPuzzle::markInRange(std::vector<unsigned> gridReferences,
	unsigned start, unsigned end, int value, unsigned& changesMade)
{
	for (unsigned i = start; i < end; i++)
	{
		if (grid[gridReferences[i]] == -1)
		{
			grid[gridReferences[i]] = value;
			changesMade++;
		}
		else if (grid[gridReferences[i]] != value)
		{
			throw CrossPuzzle::puzzle_error();
		}
	}
}

bool CrossPuzzle::isComplete(const RoworColumn& roc) const
{
	for (unsigned i = 0; i < roc.grid.size(); i++)
	{
		if (grid[roc.grid[i]] == -1)
		{
			return false;
		}
	}
	return true;
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

unsigned CrossPuzzle::removeIncompatible(RoworColumn& roc)
{
	unsigned changesMade = 0;
	
	// Check each position for a space that has recently been marked
	for (unsigned i = 0; i < roc.grid.size(); i++)
	{
		if (grid[roc.grid[i]] == 0)
		{
			// Remove due to empty spaces
			for (auto& fillPoss : roc.fillPosses)
			{
				// If the empty space is within the fill,
				// remove the possibility.
				
				auto& positions = fillPoss.possiblePositions;
				unsigned length = fillPoss.fillLength;
				unsigned originalSize = positions.size();
				
				std::erase_if(positions, [i,length](unsigned start)
				{
					return (start <= i) && (i < start + length);
				});
				
				throwIfEmpty(positions);
				
				changesMade += (originalSize - positions.size());
			}
		}
		else if (grid[roc.grid[i]] == 1)
		{
			// Remove due to filled spaces
			for (auto& fillPoss : roc.fillPosses)
			{
				// If the filled space is immediately before or after
				// a possible fill, remove the possibility.
				
				auto& positions = fillPoss.possiblePositions;
				unsigned length = fillPoss.fillLength;
				unsigned originalSize = positions.size();
				
				std::erase_if(positions, [i,length](unsigned start)
				{
					return (start == i + 1) || (start + length == i);
				});
				
				throwIfEmpty(positions);
				
				changesMade += (originalSize - positions.size());
			}
			
			// Filled space cannot fall between two adjacent fills
			for (unsigned j = 0; j < roc.fillPosses.size(); j++)
			{
				// If the first fill comes after the filled space, or
				// If the first of the two fills cannot reach the filled
				// space, the second cannot start after it.
				
				if (j == 0 || roc.fillPosses[j - 1].possiblePositions.back() +
					roc.fillPosses[j - 1].fillLength < i)
				{
					// Delete all possibilites that start after the filled
					// space.
					while (i < roc.fillPosses[j].possiblePositions.back())
					{
						roc.fillPosses[j].possiblePositions.pop_back();
						throwIfEmpty(roc.fillPosses[j].possiblePositions);
						changesMade++;
					}
				}
				
				// If the last fill ends before the filled space, or
				// If the second cannot reach it, the first cannot end
				// before it.
				
				if (j == roc.fillPosses.size() - 1 ||
					i < roc.fillPosses[j + 1].possiblePositions.front())
				{
					// Delete all possibilites that end before the filled
					// space.
					while (roc.fillPosses[j].possiblePositions.front()
						+ roc.fillPosses[j].fillLength < i)
					{
						roc.fillPosses[j].possiblePositions.pop_front();
						throwIfEmpty(roc.fillPosses[j].possiblePositions);
						changesMade++;
					}
				}
			}
		}
	}
	
	// Push back/Push forward correcting
	
	// Possible TODO: Combine this with last rule in markConsistent, or the rule above
	for (unsigned i = 1; i < roc.fillPosses.size(); i++)
	{
		unsigned furthestback = roc.fillPosses[i - 1].possiblePositions.front() +
			roc.fillPosses[i - 1].fillLength;
		while (furthestback >= roc.fillPosses[i].possiblePositions.front())
		{
			roc.fillPosses[i].possiblePositions.pop_front();
			throwIfEmpty(roc.fillPosses[i].possiblePositions);
			changesMade++;
		}
	}
	
	for (unsigned i = roc.fillPosses.size() - 1; i > 0; i--)
	{
		unsigned furthestforward = roc.fillPosses[i].possiblePositions.back();
		unsigned length = roc.fillPosses[i - 1].fillLength;
		while (furthestforward <= roc.fillPosses[i - 1].possiblePositions.back() + length)
		{
			roc.fillPosses[i - 1].possiblePositions.pop_back();
			throwIfEmpty(roc.fillPosses[i - 1].possiblePositions);
			changesMade++;
		}
	}
	
	return changesMade;
}

/*-----------------------------------------
Marks items in the grid that are consistent
with all possibilites in a RoworColumn.
-----------------------------------------*/

unsigned CrossPuzzle::markConsistent(RoworColumn& roc)
{
	unsigned changesMade = 0;
	
	// Mark filled spaces
	for (auto& fillPoss : roc.fillPosses)
	{
		// Mark every cell from the last possible start position of the fill
		// to the first possible end position of the fill. This may not mark anything.
		unsigned lastToMark = fillPoss.possiblePositions.front() + fillPoss.fillLength;
		
		markInRange(roc.grid, fillPoss.possiblePositions.back(),
			lastToMark, 1, changesMade);
	}
	
	// Mark empty spaces
	
	// Mark every cell that is before all possible start positions of the first
	// fill as empty.
	
	markInRange(roc.grid, 0, roc.fillPosses[0].possiblePositions.front(),
		0, changesMade);
	
	// Mark every cell that is after all possible end positions of the last
	// fill as empty.
	
	unsigned lastoflast = roc.fillPosses.back().possiblePositions.back()
		+ roc.fillPosses.back().fillLength;
	
	markInRange(roc.grid, lastoflast, roc.grid.size(), 0, changesMade);
	
	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned i = 1; i < roc.fillPosses.size(); i++)
	{
		unsigned lastoffirst = roc.fillPosses[i - 1].possiblePositions.back()
			+ roc.fillPosses[i - 1].fillLength;
		unsigned firstoflast = roc.fillPosses[i].possiblePositions.front();
		
		markInRange(roc.grid, lastoffirst, firstoflast, 0, changesMade);
	}
	
	return changesMade;
}

/*--------------------------------------------------------------
Calls removeIncompatible() and markConsistent(), in that order,
on each RoworColumn in lines. Returns number of changes made, or
throws puzzle_error if any RoworColumn in lines is unsolvable.
--------------------------------------------------------------*/

unsigned CrossPuzzle::removeAndMark()
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Remove and mark:" << std::endl;
	#endif
	
	unsigned changesMade = 0;
	std::erase_if(lines, [&](RoworColumn& roc)
	{
		unsigned numremoved = removeIncompatible(roc);
		unsigned nummarked  = markConsistent(roc);
		
		#ifdef CPUZZLE_DEBUG
			if (numremoved || nummarked)
			{
				std::cout << roc.ID << ":" << std::endl;
				if (numremoved)
				{
					std::cout << "Removed " << numremoved << " possibilities." << std::endl;
				}
				if (nummarked)
				{
					std::cout << "Marked " << nummarked << " cells." << std::endl;
				}
				printRoC(std::cout,roc);
			}
		#endif
		
		changesMade += (numremoved + nummarked);
		
		return isComplete(roc);
	});
	
	return changesMade;
}

/*--------------------------------------------------------------------
Creates a CrossPuzzle from information in an input file, named infile.
--------------------------------------------------------------------*/

std::istream& operator>>(std::istream& stream, CrossPuzzle& CP)
{
	stream >> CP.numcols;
	stream >> CP.numrows;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Rows: " << CP.numrows << ", Cols: " << CP.numcols << std::endl;
	#endif
	
	// Create grid, fill with -1s
	CP.grid.resize(CP.numrows * CP.numcols);
	std::fill(CP.grid.begin(), CP.grid.end(), -1);
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Hintlists:" << std::endl;
	#endif
	
	for (unsigned i = 0; i < CP.numrows; i++)
	{
		auto hintList = CrossPuzzle::getList(stream);
		
		auto references = CP.createGridReferenceLine(CP.numcols,CP.numcols * i,1);
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, references);
		#else
		CP.evaluateHintList(hintList, references, std::string("Row ") + std::to_string(i));
		#endif
	}
	
	for (unsigned i = 0; i < CP.numcols; i++)
	{
		auto hintList = CrossPuzzle::getList(stream);
		
		auto references = CP.createGridReferenceLine(CP.numrows,i,CP.numcols);
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, references);
		#else
		CP.evaluateHintList(hintList, references, std::string("Column ") + std::to_string(i));
		#endif
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Successfully read from file." << std::endl
			<< std::endl << "Puzzle:" << std::endl << CP;
	#endif
	
	return stream;
}

/*--------------------------------------
Solves this, or throws a
CrossPuzzle::puzzle_error if unsolvable.
--------------------------------------*/

void CrossPuzzle::solve()
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Entering solve:" << std::endl << "Puzzle:" << std::endl << *this;
	#endif
	
	while(removeAndMark() && !isComplete()) {}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Done with logical rules:" << std::endl;
	#endif
	
	if (isComplete())
	{
		#ifdef CPUZZLE_DEBUG
			std::cout << "Puzzle complete:" << std::endl << *this;
		#endif
		
		return;
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "\nUsing brute force:\n" << std::endl;
	#endif
	
	// find position to brute force
	unsigned pos = 0;
	while (grid[pos] != -1) ++pos;
	
	// Guess 1 first, is significantly quicker on some puzzles.
	for (int guess = 1; guess >= 0; guess--)
	{
		CrossPuzzle copy(*this);
		
		#ifdef CPUZZLE_DEBUG
			std::cout << "Guessing " << guess << " at position "
				<< pos << "(" << (pos/numrows) << "," << (pos%numrows)
				<< ")" << std::endl;
		#endif
		
		copy.grid[pos] = guess;
		
		try
		{
			copy.solve();
			std::swap(copy,*this);
			return;
		}
		catch (CrossPuzzle::puzzle_error& e) {}
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "No solution." << std::endl;
	#endif
	
	throw CrossPuzzle::puzzle_error();
}

bool CrossPuzzle::isComplete() const
{
	return lines.empty();
}

BMP_24 CrossPuzzle::bitmap() const
{
	BMP_24 soln(numrows, numcols);
	
	unsigned pos = 0;
	for (unsigned i = 0; i < numrows; i++)
	{
		// The rows in a bitmap are flipped, so when writing,
		// write to the opposite side.
		unsigned rowNum = numrows - 1 - i;
		for (unsigned j = 0; j < numcols; j++)
		{
			if (grid[pos] == 1)
			{
				soln.position(rowNum, j) = color_24::black;
			}
			// base color is white
			
			++pos;
		}
	}
	return soln;
}
