#include "cpuzzle.h"
#include "bmpMaker.h"
#include <numeric>

CrossPuzzle::RoworColumn::fill::fill(unsigned fl, unsigned start, unsigned end)
	: length(fl), candidates(end - start + 1)
{
	std::iota(candidates.begin(), candidates.end(), start);
}

/*----------------------------------------------------
Constructs a RoworColumn of length 'siz', referencing
the items in grd, with a list of hints hintList.
----------------------------------------------------*/

CrossPuzzle::RoworColumn::RoworColumn(std::vector<unsigned> grd,
	const std::list<unsigned>& hintList) : grid(grd)
{
	fills.reserve(hintList.size());
	
	unsigned sum = 0;
	for (unsigned hint : hintList) sum += hint;
	
	unsigned extraSpace = grid.size() - (sum + hintList.size() - 1);
	
	unsigned minPos = 0;
	for (unsigned hint : hintList)
	{
		fills.emplace_back(hint, minPos, minPos + extraSpace);
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
	for (auto& item : tempgrid)
	{
		item = pos;
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
			grid[ref] = cell_state::empty;
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
	
	for (const auto ref : roc.grid)
	{
		stream << grid[ref];
	}
	stream << std::endl;
	
	if (!isComplete(roc))
	{
		for (unsigned i = 0; i < roc.fills.size(); ++i)
		{
			stream << "Fill #" << i << ", length " << roc.fills[i].length << ": ";
			
			for (const auto x : roc.fills[i].candidates)
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
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		for (unsigned j = 0; j < CP.numcols; ++j)
		{
			stream << CP.grid[pos++];
		}
		stream << std::endl;
	}
	
	stream << "===========================================================" << std::endl;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, CrossPuzzle::cell_state cs)
{
	switch (cs)
	{
		case CrossPuzzle::cell_state::unknown:
			stream << "? "; break;
		case CrossPuzzle::cell_state::filled:
			stream << "X "; break;
		case CrossPuzzle::cell_state::empty:
			stream << ". "; break;
	}
	return stream;
}

#endif

/*----------------------------------
Throws a puzzle_error if L is empty.
----------------------------------*/

void CrossPuzzle::throwIfEmpty(const std::list<unsigned>& L)
{
	if (L.empty()) throw puzzle_error();
}

/*--------------------------------------------------------------------
Marks each cell in grid starting at index start and stopping before
index end to 'value', and increments changesMade for each change made.
--------------------------------------------------------------------*/

void CrossPuzzle::markInRange(std::vector<unsigned> gridReferences,
	unsigned start, unsigned end, cell_state value, unsigned& changesMade)
{
	for (unsigned i = start; i < end; ++i)
	{
		if (grid[gridReferences[i]] == cell_state::unknown)
		{
			grid[gridReferences[i]] = value;
			++changesMade;
		}
		else if (grid[gridReferences[i]] != value)
		{
			throw CrossPuzzle::puzzle_error();
		}
	}
}

bool CrossPuzzle::isComplete(const RoworColumn& roc) const
{
	for (const auto ref : roc.grid)
	{
		if (grid[ref] == cell_state::unknown)
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
	for (unsigned i = 0; i < roc.grid.size(); ++i)
	{
		if (grid[roc.grid[i]] == cell_state::empty)
		{
			// Remove due to empty spaces
			for (auto& [length,positions] : roc.fills)
			{
				// If the empty space is within the fill,
				// remove the possibility.
				
				changesMade += std::erase_if(positions, [i,length](unsigned start)
				{
					return (start <= i) && (i < start + length);
				});
				
				throwIfEmpty(positions);
			}
		}
		else if (grid[roc.grid[i]] == cell_state::filled)
		{
			// Remove due to filled spaces
			for (auto& [length,positions] : roc.fills)
			{
				// If the filled space is immediately before or after
				// a possible fill, remove the possibility.
				
				changesMade += std::erase_if(positions, [i,length](unsigned start)
				{
					return (start == i + 1) || (start + length == i);
				});
				
				throwIfEmpty(positions);
			}
			
			// Filled space cannot fall between two adjacent fills
			for (unsigned j = 0; j < roc.fills.size(); ++j)
			{
				// If the first fill comes after the filled space, or
				// If the first of the two fills cannot reach the filled
				// space, the second cannot start after it.
				
				if (j == 0 || roc.fills[j - 1].candidates.back() +
					roc.fills[j - 1].length < i)
				{
					// Delete all possibilites that start after the filled
					// space.
					while (i < roc.fills[j].candidates.back())
					{
						roc.fills[j].candidates.pop_back();
						throwIfEmpty(roc.fills[j].candidates);
						++changesMade;
					}
				}
				
				// If the last fill ends before the filled space, or
				// If the second cannot reach it, the first cannot end
				// before it.
				
				if (j == roc.fills.size() - 1 ||
					i < roc.fills[j + 1].candidates.front())
				{
					// Delete all possibilites that end before the filled
					// space.
					while (roc.fills[j].candidates.front()
						+ roc.fills[j].length < i)
					{
						roc.fills[j].candidates.pop_front();
						throwIfEmpty(roc.fills[j].candidates);
						++changesMade;
					}
				}
			}
		}
	}
	
	// Push back/Push forward correcting
	
	// Possible TODO: Combine this with last rule in markConsistent, or the rule above
	for (unsigned i = 1; i < roc.fills.size(); ++i)
	{
		unsigned furthestback = roc.fills[i - 1].candidates.front() +
			roc.fills[i - 1].length;
		while (furthestback >= roc.fills[i].candidates.front())
		{
			roc.fills[i].candidates.pop_front();
			throwIfEmpty(roc.fills[i].candidates);
			++changesMade;
		}
	}
	
	for (unsigned i = roc.fills.size() - 1; i > 0; --i)
	{
		unsigned furthestforward = roc.fills[i].candidates.back();
		unsigned length = roc.fills[i - 1].length;
		while (furthestforward <= roc.fills[i - 1].candidates.back() + length)
		{
			roc.fills[i - 1].candidates.pop_back();
			throwIfEmpty(roc.fills[i - 1].candidates);
			++changesMade;
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
	for (auto& [length,positions] : roc.fills)
	{
		// Mark every cell from the last possible start position of the fill
		// to the first possible end position of the fill. This may not mark anything.
		markInRange(roc.grid, positions.back(),
			positions.front() + length, cell_state::filled, changesMade);
	}
	
	// Mark empty spaces
	
	// Mark every cell that is before all possible start positions of the first
	// fill as empty.
	
	markInRange(roc.grid, 0, roc.fills[0].candidates.front(),
		cell_state::empty, changesMade);
	
	// Mark every cell that is after all possible end positions of the last
	// fill as empty.
	
	unsigned lastoflast = roc.fills.back().candidates.back()
		+ roc.fills.back().length;
	
	markInRange(roc.grid, lastoflast, roc.grid.size(), cell_state::empty, changesMade);
	
	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned i = 1; i < roc.fills.size(); ++i)
	{
		unsigned lastoffirst = roc.fills[i - 1].candidates.back()
			+ roc.fills[i - 1].length;
		unsigned firstoflast = roc.fills[i].candidates.front();
		
		markInRange(roc.grid, lastoffirst, firstoflast, cell_state::empty, changesMade);
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
	stream >> CP.numcols >> CP.numrows;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Rows: " << CP.numrows << ", Cols: " << CP.numcols << std::endl;
	#endif
	
	// Create grid, fill with "unknown"
	CP.grid.resize(CP.numrows * CP.numcols);
	std::fill(CP.grid.begin(), CP.grid.end(), CrossPuzzle::cell_state::unknown);
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Hintlists:" << std::endl;
	#endif
	
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		auto hintList = CrossPuzzle::getList(stream);
		
		auto references = CP.createGridReferenceLine(CP.numcols,CP.numcols * i,1);
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, references);
		#else
		CP.evaluateHintList(hintList, references, std::string("Row ") + std::to_string(i));
		#endif
	}
	
	for (unsigned i = 0; i < CP.numcols; ++i)
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
	while (grid[pos] != cell_state::unknown) ++pos;
	
	// Guess filled first, is significantly quicker on some puzzles.
	for (auto guess : { cell_state::filled, cell_state::empty })
	{
		CrossPuzzle copy(*this);
		
		#ifdef CPUZZLE_DEBUG
			std::cout << "Guessing "
				<< (guess == cell_state::filled ? "filled" : "empty")
				<< " at position " << pos << "(" << (pos / numrows) << ","
				<< (pos % numrows) << ")" << std::endl;
		#endif
		
		copy.grid[pos] = guess;
		
		try
		{
			copy.solve();
			std::swap(copy,*this);
			return;
		}
		catch (puzzle_error&) {}
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "No solution." << std::endl;
	#endif
	
	throw puzzle_error();
}

bool CrossPuzzle::isComplete() const
{
	return lines.empty();
}

BMP_24 CrossPuzzle::bitmap() const
{
	BMP_24 soln(numrows, numcols);
	
	unsigned pos = 0;
	for (unsigned i = 0; i < numrows; ++i)
	{
		// The rows in a bitmap are flipped, so when writing,
		// write to the opposite side.
		unsigned rowNum = numrows - 1 - i;
		for (unsigned j = 0; j < numcols; ++j)
		{
			if (grid[pos] == cell_state::filled)
			{
				soln.position(rowNum, j) = color_24::black;
			}
			// base color is white
			
			++pos;
		}
	}
	return soln;
}
