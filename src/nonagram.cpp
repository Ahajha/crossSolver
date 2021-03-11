#include "nonagram.hpp"
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

nonagram::line::line(const std::vector<unsigned>& grd,
	const std::vector<unsigned>& hintList) : grid(grd.size())
{
	for (unsigned i = 0; i < grd.size(); ++i)
	{
		grid[i] = { grd[i], false };
	}
	
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

std::vector<unsigned> nonagram::getList(std::istream& in)
{
	unsigned temp;
	std::vector<unsigned> L;
	
	// There will always be at least one number
	in >> temp;
	L.push_back(temp);
	
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
		L.pop_back();
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

// If hintList is empty, fills in the respective line with empty cells.
// Otherwise, constructs a line object at index idx. References refers
// to the indexes in grid to which this line would refer to.
void nonagram::evaluateHintList(const std::vector<unsigned>& hintList,
#ifndef CPUZZLE_DEBUG
	std::vector<unsigned>&& references, unsigned idx)
#else
	std::vector<unsigned>&& references, unsigned idx, std::string&& ID)
#endif
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "    " << ID << ": ";
		for (unsigned hint : hintList) std::cout << hint << ' ';
		std::cout << '\n';
	#endif
	
	if (hintList.empty())
	{
		for (unsigned ref : references)
		{
			grid[ref] = cell_state::empty;
		}
		
		// Go ahead and report this line as solved
		--lines_to_solve;
	}
	else
	{
		lines[idx].emplace(std::move(references), hintList);
		#ifdef CPUZZLE_DEBUG
		lines[idx]->ID = std::move(ID);
		#endif
	}
}

#ifdef CPUZZLE_DEBUG
void nonagram::print_line(std::ostream& stream, const line& lin) const
{
	stream << (isComplete(lin) ? "Complete." : "Incomplete.") << " Grid:\n";
	
	for (const auto ref : lin.grid)
	{
		stream << grid[ref.first];
	}
	stream << '\n';
	
	if (!isComplete(lin))
	{
		for (unsigned i = 0; i < lin.fills.size(); ++i)
		{
			stream << "Fill #" << i << ", length " << lin.fills[i].length << ": ";
			
			for (const auto x : lin.fills[i].candidates)
			{
				stream << x << ' ';
			}
			
			stream << '\n';
		}
	}
	stream << '\n';
}

std::ostream& operator<<(std::ostream& stream, const nonagram& CP)
{
	stream << "===========================================================\n";
	stream << "Rows: " << CP.numrows << ", Columns: " << CP.numcols << "\n\n";
	
	// If CP is complete, this will print nothing
	for (auto& lin : CP.lines)
	{
		if (lin)
		{
			stream << lin->ID << ": ";
			CP.print_line(stream,*lin);
		}
	}
	
	unsigned pos = 0;
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		for (unsigned j = 0; j < CP.numcols; ++j)
		{
			stream << CP.grid[pos++];
		}
		stream << '\n';
	}
	
	stream << "===========================================================" << '\n';
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

void nonagram::throwIfEmpty(const std::deque<unsigned>& L)
{
	if (L.empty()) throw puzzle_error();
}

/*--------------------------------------------------------------------
Marks each cell in grid starting at index start and stopping before
index end to 'value', and increments changesMade for each change made.
--------------------------------------------------------------------*/

void nonagram::markInRange(const std::vector<std::pair<unsigned,bool>>& gridReferences,
	unsigned start, unsigned end, cell_state value, unsigned& changesMade)
{
	for (unsigned i = start; i < end; ++i)
	{
		if (grid[gridReferences[i].first] == cell_state::unknown)
		{
			grid[gridReferences[i].first] = value;
			++changesMade;
		}
		else if (grid[gridReferences[i].first] != value)
		{
			throw nonagram::puzzle_error();
		}
	}
}

bool nonagram::isComplete(const line& lin) const
{
	for (const auto& ref : lin.grid)
	{
		if (grid[ref.first] == cell_state::unknown)
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
		if (grid[lin.grid[i].first] == cell_state::empty)
		{
			// If this rule has not been applied yet, do so.
			if (!lin.grid[i].second)
			{
				lin.grid[i].second = true;
				
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
		}
		else if (grid[lin.grid[i].first] == cell_state::filled)
		{
			// If this rule has not been applied yet, do so.
			if (!lin.grid[i].second)
			{
				lin.grid[i].second = true;
				
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
			}
			
			// These rules should be done regardless, as they may change over time.
			
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
	
	// Possible TODO: Combine these with last rule in markConsistent, or the rule above
	
	// Push-forward correcting. This loop goes forwards, since pushing a fill
	// forward can affect fills later down the line.
	for (unsigned i = 1; i < lin.fills.size(); ++i)
	{
		// Suppose a given fill is placed in the first available position. Any
		// cells it occupies cannot possibly be filled by the next fill, or the
		// cell immediately after, since there must be an empty space.
		
		// The cell immediately after the fill in its first available position.
		const unsigned pos_after = lin.fills[i - 1].candidates.front() +
			lin.fills[i - 1].length;
		
		// This can be tested efficiently, since pos_after must be strictly
		// before the starting cell of the next fill, so remove candidates of
		// the next fill that start before or on the pos_after cell.
		while (pos_after >= lin.fills[i].candidates.front())
		{
			lin.fills[i].candidates.pop_front();
			throwIfEmpty(lin.fills[i].candidates);
			++changesMade;
		}
	}
	
	// Push-back correcting. This loop goes backwards, since pushing a fill
	// backwards can affect fills before it.
	for (unsigned i = lin.fills.size() - 1; i > 0; --i)
	{
		// Suppose a given fill is placed in the last available position. Any
		// cells it occupies cannot possibly be filled by the previous fill, or
		// the cell immediately before, since there must be an empty space.
		
		// The first cell of the fill in its last available position.
		const unsigned first_cell = lin.fills[i].candidates.back();
		
		// Value grabbed for brevity
		const unsigned length = lin.fills[i - 1].length;
		
		// This can be tested efficiently, since first_cell must be strictly
		// after the cell that is one past the end of the previous fill (the
		// value computed by the right-hand side of the following inequality).
		// Remove candidates of the previous fill that end on or after the
		// first_cell cell.
		while (first_cell <= lin.fills[i - 1].candidates.back() + length)
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
		std::cout << "Remove and mark:\n";
	#endif
	
	unsigned changesMade = 0;
	for (std::optional<line>& lin : lines)
	{
		if (!lin) continue;
		
		unsigned numremoved = removeIncompatible(*lin);
		unsigned nummarked  = markConsistent(*lin);
		
		#ifdef CPUZZLE_DEBUG
			if (numremoved || nummarked)
			{
				std::cout << lin->ID << ":\n";
				if (numremoved)
				{
					std::cout << "Removed " << numremoved << " possibilities.\n";
				}
				if (nummarked)
				{
					std::cout << "Marked " << nummarked << " cells.\n";
				}
				print_line(std::cout,*lin);
			}
		#endif
		
		changesMade += (numremoved + nummarked);
		
		// If the line is solved, remove it and reduce the number of
		// remaining lines to solve.
		if (isComplete(*lin))
		{
			lin.reset();
			--lines_to_solve;
		}
	}
	
	return changesMade;
}

/*-----------------------------------------------------------------
Creates a nonagram from information in an input file, named infile.
-----------------------------------------------------------------*/

std::istream& operator>>(std::istream& stream, nonagram& CP)
{
	stream >> CP.numcols >> CP.numrows;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Rows: " << CP.numrows << ", Cols: " << CP.numcols << '\n';
	#endif
	
	CP.lines.resize(CP.lines_to_solve = CP.numcols + CP.numrows);
	
	// Create grid, fill with "unknown"
	CP.grid.resize(CP.numrows * CP.numcols);
	std::fill(CP.grid.begin(), CP.grid.end(), nonagram::cell_state::unknown);
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Hintlists:\n";
	#endif
	
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		auto hintList = nonagram::getList(stream);
		
		auto references = CP.createGridReferenceLine(CP.numcols,CP.numcols * i,1);
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, std::move(references), i);
		#else
		CP.evaluateHintList(hintList, std::move(references), i,
			std::string("Row ") + std::to_string(i));
		#endif
	}
	
	for (unsigned i = 0; i < CP.numcols; ++i)
	{
		auto hintList = nonagram::getList(stream);
		
		auto references = CP.createGridReferenceLine(CP.numrows,i,CP.numcols);
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, std::move(references), CP.numrows + i);
		#else
		CP.evaluateHintList(hintList, std::move(references), CP.numrows + i,
			std::string("Column ") + std::to_string(i));
		#endif
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Successfully read from file.\n\nPuzzle:\n" << CP;
	#endif
	
	return stream;
}

/*------------------------------------------------------------
Solves this, or throws a nonagram::puzzle_error if unsolvable.
------------------------------------------------------------*/

void nonagram::solve()
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Entering solve:\nPuzzle:\n" << *this;
	#endif
	
	while(removeAndMark() && !isComplete()) {}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Done with logical rules:\n";
	#endif
	
	if (isComplete())
	{
		#ifdef CPUZZLE_DEBUG
			std::cout << "Puzzle complete:\n" << *this;
		#endif
		
		return;
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "\nUsing brute force, " << lines_to_solve << " lines left"
			" to solve:\n\n";
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
				<< (pos % numrows) << ")\n";
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
		std::cout << "No solution.\n";
	#endif
	
	throw puzzle_error();
}

bool nonagram::isComplete() const
{
	return lines_to_solve == 0;
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
