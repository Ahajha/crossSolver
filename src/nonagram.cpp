#include "nonagram.hpp"
#include <numeric>
#include <ranges>
#include <algorithm>

nonagram::line::fill::fill(unsigned fl, unsigned start, unsigned end)
	: length(fl), candidates(end - start + 1)
{
	std::iota(candidates.begin(), candidates.end(), start);
}

/*--------------------------------------------------------------------
Constructs a line of with length grd.size(), referencing the items in
grd, with a given hint list. Offset is the offset of the index to the
opposite line, should be 0 if this line is a column, as it referencing
rows, and numrows if this line is a row, as it references columns.
--------------------------------------------------------------------*/

nonagram::line::line(auto&& grd, const std::vector<unsigned>& hintList,
	unsigned offset) : grid(grd.size()), needs_line_solving(true)
{
	for (unsigned i = 0; i < grd.size(); ++i)
	{
		grid[i] = { grd[i], i + offset, false };
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

/*-------------------------------------------------
Returns one line of unsigned integers read from in.
-------------------------------------------------*/

void nonagram::getList(std::istream& in, std::vector<unsigned>& L)
{
	unsigned temp;
	
	// There will always be at least one number
	in >> temp;
	L.push_back(temp);
	
	for (int c; (c = in.peek()) != '\n' && in;)
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
}

/*-----------------------------------------------------------------
If hintList is empty, fills in the respective line with empty
cells. Otherwise, constructs a line object at index idx. References
refers to the indexes in grid to which this line would refer to.
-----------------------------------------------------------------*/

void nonagram::evaluateHintList(const std::vector<unsigned>& hintList,
#ifndef CPUZZLE_DEBUG
	auto&& references, unsigned idx, unsigned offset)
#else
	auto&& references, unsigned idx, unsigned offset, std::string&& ID)
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
		lines[idx].emplace(references, hintList, offset);
		#ifdef CPUZZLE_DEBUG
		lines[idx]->ID = std::move(ID);
		#endif
	}
}

#ifdef CPUZZLE_DEBUG
std::ostream& operator<<(std::ostream& stream, const nonagram::line& lin)
{
	stream << lin.ID << ". Fills:\n";
	
	for (const auto& [length, candidates] : lin.fills)
	{
		stream << "length " << length << ": ";
		
		for (const auto x : candidates)
		{
			stream << x << ' ';
		}
		
		stream << '\n';
	}
	
	return stream << '\n';
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
			stream << *lin;
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
	
	stream << "===========================================================\n";
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

/*---------------------------------------------------------
Marks each cell in grid starting at index start and
stopping before index end to 'value'. Throws a puzzle_error
if any change is inconsistent with the existing value.
---------------------------------------------------------*/

void nonagram::markInRange(const std::vector<line::cell>& gridReferences,
	unsigned start, unsigned end, cell_state value)
{
	for (unsigned i = start; i < end; ++i)
	{
		if (grid[gridReferences[i].ref_index] == cell_state::unknown)
		{
			grid[gridReferences[i].ref_index] = value;
			lines[gridReferences[i].opposite_line]->needs_line_solving = true;
		}
		else if (grid[gridReferences[i].ref_index] != value)
		{
			throw puzzle_error();
		}
	}
}

bool nonagram::isComplete(const line& lin) const
{
	for (const auto& ref : lin.grid)
	{
		if (grid[ref.ref_index] == cell_state::unknown)
		{
			return false;
		}
	}
	return true;
}

/*-------------------------------------------------
Removes incompatible candidates from a line. If a
fill has no more candidates, throws a puzzle_error.
-------------------------------------------------*/

void nonagram::removeIncompatible(line& lin)
{
	// Check each position for a space that has recently been marked
	for (unsigned i = 0; i < lin.grid.size(); ++i)
	{
		if (grid[lin.grid[i].ref_index] == cell_state::empty)
		{
			// If this rule has not been applied yet, do so.
			if (!lin.grid[i].rules_used)
			{
				lin.grid[i].rules_used = true;
				
				// Remove due to empty spaces
				for (auto& [length,positions] : lin.fills)
				{
					// If the empty space is within the fill,
					// remove the possibility.
					
					std::erase_if(positions, [i,length](unsigned start)
					{
						return (start <= i) && (i < start + length);
					});
					
					throwIfEmpty(positions);
				}
			}
		}
		else if (grid[lin.grid[i].ref_index] == cell_state::filled)
		{
			// If this rule has not been applied yet, do so.
			if (!lin.grid[i].rules_used)
			{
				lin.grid[i].rules_used = true;
				
				// Remove due to filled spaces
				for (auto& [length,positions] : lin.fills)
				{
					// If the filled space is immediately before or after
					// a possible fill, remove the possibility.
					
					std::erase_if(positions, [i,length](unsigned start)
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
		}
	}
}

/*--------------------------------------------------------------------
Marks items in the grid that are consistent with all possibilites in
a line. Important property: No cells marked will cause any fill
candidates within this line to be invalidated (in other words, there
is no need to call removeIncompatible after markConsistent is called).
--------------------------------------------------------------------*/

void nonagram::markConsistent(line& lin)
{
	// Mark filled spaces
	
	// This rule does satisfy the mentioned property, however it isn't obvious.
	// First note that all marked cells are between the last position of the
	// first possible location, and the first position of the last possible
	// location. There will certainly not be a candidate from this fill that
	// gets removed, and there cannot be a candidate removed from an adjacent
	// fill, due to push back/forward correcting (it should be obvious from
	// this point that no other fills can be modified).
	for (auto& [length,positions] : lin.fills)
	{
		// Mark every cell from the last possible start position of the fill to
		// the first possible end position of the fill. This may not mark anything.
		markInRange(lin.grid, positions.back(), positions.front() + length,
			cell_state::filled);
	}
	
	// Mark empty spaces
	
	// These rules obviously satisfy the stated property, as no candidates in
	// any of these touch the cells being marked, which is why they are being
	// marked as empty in the first place.
	
	// Mark every cell that is before all possible start positions of the first
	// fill as empty.
	
	markInRange(lin.grid, 0, lin.fills[0].candidates.front(), cell_state::empty);
	
	// Mark every cell that is after all possible end positions of the last
	// fill as empty.
	
	unsigned lastoflast = lin.fills.back().candidates.back()
		+ lin.fills.back().length;
	
	markInRange(lin.grid, lastoflast, lin.grid.size(), cell_state::empty);
	
	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned i = 1; i < lin.fills.size(); ++i)
	{
		unsigned lastoffirst = lin.fills[i - 1].candidates.back()
			+ lin.fills[i - 1].length;
		unsigned firstoflast = lin.fills[i].candidates.front();
		
		markInRange(lin.grid, lastoffirst, firstoflast, cell_state::empty);
	}
}

/*-------------------------------------------------------------
Calls removeIncompatible() and markConsistent(), in that order,
on each line. Throws a puzzle_error if any line is unsolvable.
-------------------------------------------------------------*/

void nonagram::line_solve()
{
	// Index of the last line that was line solved in this method call,
	// starts as an unreachable value. It is assumed that at least one line
	// is marked for line solving.
	unsigned last_solved = lines.size();
	while (true)
	{
		for (unsigned i = 0; i < lines.size(); ++i)
		{
			if (i == last_solved) return;
			
			auto& lin = lines[i];
			
			if (!lin || !lin->needs_line_solving) continue;
			
			removeIncompatible(*lin);
			markConsistent(*lin);
			
			// If the line is solved, remove it and reduce the number of
			// remaining lines to solve.
			if (isComplete(*lin))
			{
				lin.reset();
				--lines_to_solve;
			}
			else
			{
				lin->needs_line_solving = false;
			}
			
			last_solved = i;
		}
	}
}

std::istream& operator>>(std::istream& stream, nonagram& CP)
{
	stream >> CP.numcols >> CP.numrows;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Rows: " << CP.numrows << ", Cols: " << CP.numcols << '\n';
	#endif
	
	CP.lines.resize(CP.lines_to_solve = CP.numcols + CP.numrows);
	
	// Create grid, fill with "unknown"
	CP.grid.resize(CP.numrows * CP.numcols);
	std::ranges::fill(CP.grid, nonagram::cell_state::unknown);
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Hintlists:\n";
	#endif
	
	// Used to help create views of indexes. Each item in an iota_view
	// will be mapped to an index, such that all indexes in the view
	// represent a row or a column, based on what offset and inc are.
	// A functor is a bit cleaner here, since the formula is the same, and
	// avoids having to create two very similar lambdas.
	struct ref_creator
	{
		unsigned offset, inc;
		
		unsigned operator()(unsigned x) const { return x * inc + offset; }
	};
	
	std::vector<unsigned> hintList;
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		nonagram::getList(stream, hintList);
		
		auto references = std::ranges::iota_view(0u,CP.numcols)
			| std::views::transform(ref_creator{CP.numcols * i, 1});
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, references, i, CP.numrows);
		#else
		CP.evaluateHintList(hintList, references, i, CP.numrows,
			std::string("Row ") + std::to_string(i));
		#endif
		
		hintList.clear();
	}
	
	for (unsigned i = 0; i < CP.numcols; ++i)
	{
		nonagram::getList(stream, hintList);
		
		auto references = std::ranges::iota_view(0u,CP.numrows)
			| std::views::transform(ref_creator{i, CP.numcols});
		
		#ifndef CPUZZLE_DEBUG
		CP.evaluateHintList(hintList, references, CP.numrows + i, 0);
		#else
		CP.evaluateHintList(hintList, references, CP.numrows + i, 0,
			std::string("Column ") + std::to_string(i));
		#endif
		
		hintList.clear();
	}
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Successfully read from file.\n";
	#endif
	
	return stream;
}

void nonagram::solve()
{
	#ifdef CPUZZLE_DEBUG
		std::cout << "Entering solve:\nPuzzle:\n" << *this << "Line solving:\n";
	#endif
	
	line_solve();
	
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
	
	// Mark the affected row and column, respectfully, as needing line solving.
	lines[pos / numcols]->needs_line_solving = true;
	lines[numrows + pos % numcols]->needs_line_solving = true;
	
	// For now, naively guess filled. This guess will be improved in the future.
	auto guess = cell_state::filled;
	
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
	
	guess = (guess == cell_state::filled) ? cell_state::empty : cell_state::filled;
	
	#ifdef CPUZZLE_DEBUG
		std::cout << "Guess at position " << pos << "(" << (pos / numrows)
			<< "," << (pos % numrows) << ") was incorrect, trying "
			<< (guess == cell_state::filled ? "filled" : "empty")
			<< " instead\n";
	#endif
	
	// No need to use the copy anymore, if this solves the puzzle the solution
	// is already in place, if not, the puzzle is "junk" anyways.
	grid[pos] = guess;
	
	// Since this is the last attempt, if this throws, let the error escalate.
	solve();
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
				soln(rowNum, j) = color_24_consts::black;
			}
			// base color is white
			
			++pos;
		}
	}
	return soln;
}
