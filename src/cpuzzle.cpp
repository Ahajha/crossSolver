#include "cpuzzle.h"
#include "bmpMaker.h"
#include <numeric>

nonagram::line::fill::fill(unsigned fl, unsigned start, unsigned end)
	: length(fl), candidates(end - start + 1)
{
	std::iota(candidates.begin(), candidates.end(), start);
}

/*----------------------------------------------
Constructs a line of length 'siz', referencing
the items in grd, with a list of hints hintList.
----------------------------------------------*/

nonagram::line::line(std::vector<unsigned> grd,
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

std::list<unsigned> nonagram::getList(std::istream& in)
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

std::vector<unsigned> nonagram::createGridReferenceLine(unsigned size,
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

void nonagram::evaluateHintList(std::list<unsigned> hintList,
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
void nonagram::print_line(std::ostream& stream, const line& lin) const
{
	if(isComplete(lin))
	{
		stream << "Complete. Grid:" << std::endl;
	}
	else
	{
		stream << "Incomplete. Grid:" << std::endl;
	}
	
	for (const auto ref : lin.grid)
	{
		stream << grid[ref];
	}
	stream << std::endl;
	
	if (!isComplete(lin))
	{
		for (unsigned i = 0; i < lin.fills.size(); ++i)
		{
			stream << "Fill #" << i << ", length " << lin.fills[i].length << ": ";
			
			for (const auto x : lin.fills[i].candidates)
			{
				stream << x << ' ';
			}
			
			stream << std::endl;
		}
	}
	stream << std::endl;
}

std::ostream& operator<<(std::ostream& stream, const nonagram& CP)
{
	stream << "===========================================================" << std::endl;
	stream << "Rows: " << CP.numrows << ", Columns: " << CP.numcols << std::endl << std::endl;
	
	// If CP is complete, this will print nothing
	for (auto& lin : CP.lines)
	{
		stream << lin.ID << ": ";
		CP.print_line(stream,lin);
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

std::ostream& operator<<(std::ostream& stream, nonagram::cell_state cs)
{
	switch (cs)
	{
		case nonagram::cell_state::unknown:
			stream << "? "; break;
		case nonagram::cell_state::filled:
			stream << "X "; break;
		case nonagram::cell_state::empty:
			stream << ". "; break;
	}
	return stream;
}

#endif

/*----------------------------------
Throws a puzzle_error if L is empty.
----------------------------------*/

void nonagram::throwIfEmpty(const std::list<unsigned>& L)
{
	if (L.empty()) throw puzzle_error();
}

/*--------------------------------------------------------------------
Marks each cell in grid starting at index start and stopping before
index end to 'value', and increments changesMade for each change made.
--------------------------------------------------------------------*/

void nonagram::markInRange(std::vector<unsigned> gridReferences,
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
			throw nonagram::puzzle_error();
		}
	}
}

bool nonagram::isComplete(const line& lin) const
{
	for (const auto ref : lin.grid)
	{
		if (grid[ref] == cell_state::unknown)
		{
			return false;
		}
	}
	return true;
}

/*-------------------------------------------------
Removes incompatible candidates from a line. If a
fill has no more candidates, throws a puzzle_error,
otherwise returns the number of candidates removed.
-------------------------------------------------*/

// TODO: Some rules added for the first and last fills can be condensed by
// making "empty" first and last fills (i.e. cell 0 to -1, length 0 for the
// beginning). Need to double check, but it seems that all of the special
// case rules would collapse into other rules.

unsigned nonagram::removeIncompatible(line& lin)
{
	unsigned changesMade = 0;
	
	// Check each position for a space that has recently been marked
	for (unsigned i = 0; i < lin.grid.size(); ++i)
	{
		if (grid[lin.grid[i]] == cell_state::empty)
		{
			// Remove due to empty spaces
			for (auto& [length,positions] : lin.fills)
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
		else if (grid[lin.grid[i]] == cell_state::filled)
		{
			// Remove due to filled spaces
			for (auto& [length,positions] : lin.fills)
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
			for (unsigned j = 0; j < lin.fills.size(); ++j)
			{
				// If the first fill comes after the filled space, or
				// If the first of the two fills cannot reach the filled
				// space, the second cannot start after it.
				
				if (j == 0 || lin.fills[j - 1].candidates.back() +
					lin.fills[j - 1].length < i)
				{
					// Delete all possibilites that start after the filled
					// space.
					while (i < lin.fills[j].candidates.back())
					{
						lin.fills[j].candidates.pop_back();
						throwIfEmpty(lin.fills[j].candidates);
						++changesMade;
					}
				}
				
				// If the last fill ends before the filled space, or
				// If the second cannot reach it, the first cannot end
				// before it.
				
				if (j == lin.fills.size() - 1 ||
					i < lin.fills[j + 1].candidates.front())
				{
					// Delete all possibilites that end before the filled
					// space.
					while (lin.fills[j].candidates.front()
						+ lin.fills[j].length < i)
					{
						lin.fills[j].candidates.pop_front();
						throwIfEmpty(lin.fills[j].candidates);
						++changesMade;
					}
				}
			}
		}
	}
	
	// Push back/Push forward correcting
	
	// Possible TODO: Combine this with last rule in markConsistent, or the rule above
	for (unsigned i = 1; i < lin.fills.size(); ++i)
	{
		unsigned furthestback = lin.fills[i - 1].candidates.front() +
			lin.fills[i - 1].length;
		while (furthestback >= lin.fills[i].candidates.front())
		{
			lin.fills[i].candidates.pop_front();
			throwIfEmpty(lin.fills[i].candidates);
			++changesMade;
		}
	}
	
	for (unsigned i = lin.fills.size() - 1; i > 0; --i)
	{
		unsigned furthestforward = lin.fills[i].candidates.back();
		unsigned length = lin.fills[i - 1].length;
		while (furthestforward <= lin.fills[i - 1].candidates.back() + length)
		{
			lin.fills[i - 1].candidates.pop_back();
			throwIfEmpty(lin.fills[i - 1].candidates);
			++changesMade;
		}
	}
	
	return changesMade;
}

/*-----------------------------------------
Marks items in the grid that are consistent
with all possibilites in a line.
-----------------------------------------*/

unsigned nonagram::markConsistent(line& lin)
{
	unsigned changesMade = 0;
	
	// Mark filled spaces
	for (auto& [length,positions] : lin.fills)
	{
		// Mark every cell from the last possible start position of the fill
		// to the first possible end position of the fill. This may not mark anything.
		markInRange(lin.grid, positions.back(),
			positions.front() + length, cell_state::filled, changesMade);
	}
	
	// Mark empty spaces
	
	// Mark every cell that is before all possible start positions of the first
	// fill as empty.
	
	markInRange(lin.grid, 0, lin.fills[0].candidates.front(),
		cell_state::empty, changesMade);
	
	// Mark every cell that is after all possible end positions of the last
	// fill as empty.
	
	unsigned lastoflast = lin.fills.back().candidates.back()
		+ lin.fills.back().length;
	
	markInRange(lin.grid, lastoflast, lin.grid.size(), cell_state::empty, changesMade);
	
	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned i = 1; i < lin.fills.size(); ++i)
	{
		unsigned lastoffirst = lin.fills[i - 1].candidates.back()
			+ lin.fills[i - 1].length;
		unsigned firstoflast = lin.fills[i].candidates.front();
		
		markInRange(lin.grid, lastoffirst, firstoflast, cell_state::empty, changesMade);
	}
	
	return changesMade;
}

/*-----------------------------------------------------------
Calls removeIncompatible() and markConsistent(), in that
order, on each line in lines. Returns number of changes made,
or throws puzzle_error if any line in lines is unsolvable.
-----------------------------------------------------------*/

unsigned nonagram::removeAndMark()
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Remove and mark:" << std::endl;
	#endif
	
	unsigned changesMade = 0;
	std::erase_if(lines, [&](line& lin)
	{
		unsigned numremoved = removeIncompatible(lin);
		unsigned nummarked  = markConsistent(lin);
		
		#ifdef CPUZZLE_DEBUG
			if (numremoved || nummarked)
			{
				std::cout << lin.ID << ":" << std::endl;
				if (numremoved)
				{
					std::cout << "Removed " << numremoved << " possibilities." << std::endl;
				}
				if (nummarked)
				{
					std::cout << "Marked " << nummarked << " cells." << std::endl;
				}
				print_line(std::cout,lin);
			}
		#endif
		
		changesMade += (numremoved + nummarked);
		
		return isComplete(lin);
	});
	
	return changesMade;
}

/*-----------------------------------------------------------------
Creates a nonagram from information in an input file, named infile.
-----------------------------------------------------------------*/

std::istream& operator>>(std::istream& stream, nonagram& CP)
{
	stream >> CP.numcols >> CP.numrows;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Rows: " << CP.numrows << ", Cols: " << CP.numcols << std::endl;
	#endif
	
	// Create grid, fill with "unknown"
	CP.grid.resize(CP.numrows * CP.numcols);
	std::fill(CP.grid.begin(), CP.grid.end(), nonagram::cell_state::unknown);
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Hintlists:" << std::endl;
	#endif
	
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		auto hintList = nonagram::getList(stream);
		
		auto references = CP.createGridReferenceLine(CP.numcols,CP.numcols * i,1);
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, references);
		#else
		CP.evaluateHintList(hintList, references, std::string("Row ") + std::to_string(i));
		#endif
	}
	
	for (unsigned i = 0; i < CP.numcols; ++i)
	{
		auto hintList = nonagram::getList(stream);
		
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

/*------------------------------------------------------------
Solves this, or throws a nonagram::puzzle_error if unsolvable.
------------------------------------------------------------*/

void nonagram::solve()
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
		nonagram copy(*this);
		
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

bool nonagram::isComplete() const
{
	return lines.empty();
}

BMP_24 nonagram::bitmap() const
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
