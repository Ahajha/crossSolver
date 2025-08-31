#include "nonagram.hpp"

#include <algorithm>
#include <numeric>

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

nonagram::line::line(index_generator<>&& refs, const std::vector<unsigned>& hintList, bool is_r)
	: grid(refs), needs_line_solving(true), is_row(is_r)
{
	fills.reserve(hintList.size());

	const auto sum = std::accumulate(hintList.begin(), hintList.end(), 0U);

	unsigned extraSpace = grid.size() - (sum + static_cast<unsigned>(hintList.size()) - 1);

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

static void getList(std::istream& in, std::vector<unsigned>& L)
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

void nonagram::evaluateHintList(index_generator<>&& refs, const std::vector<unsigned>& hintList,
                                unsigned idx, bool is_r)
{
#ifdef CPUZZLE_DEBUG
	for (unsigned hint : hintList)
		std::cout << hint << ' ';
	std::cout << '\n';
#endif

	if (hintList.empty())
	{
		for (unsigned ref : refs)
		{
			grid[ref] = cell_state::empty;
		}

		// Go ahead and report this line as solved
		--lines_to_solve;
	}
	else
	{
		lines[idx].emplace(std::forward<index_generator<>>(refs), hintList, is_r);
	}
}

#ifdef CPUZZLE_DEBUG
std::ostream& operator<<(std::ostream& stream, const nonagram::line& lin)
{
	stream << " Fills:\n";

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
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		if (CP.lines[i])
		{
			stream << "Row " << i << ":" << *(CP.lines[i]);
		}
	}

	for (unsigned i = CP.numrows; i < CP.lines.size(); ++i)
	{
		if (CP.lines[i])
		{
			stream << "Column " << (i - CP.numrows) << ":" << *(CP.lines[i]);
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
		stream << "? ";
		break;
	case nonagram::cell_state::filled:
		stream << "X ";
		break;
	case nonagram::cell_state::empty:
		stream << ". ";
		break;
	}
	return stream;
}
#endif

/*---------------------------------------------------------
Marks each cell in grid starting at index start and
stopping before index end to 'value'. Returns false
if any change is inconsistent with the existing value.
---------------------------------------------------------*/

bool nonagram::markInRange(line& lin, unsigned start, unsigned end, cell_state value)
{
	const auto end_iter = lin.grid.begin() + end;
	for (auto iter = lin.grid.begin() + start; iter < end_iter; ++iter)
	{
		const unsigned ref_index = *iter;
		if (grid[ref_index] == cell_state::unknown)
		{
			grid[ref_index] = value;

			const unsigned opposite_index =
				lin.is_row ? (ref_index / numcols) : (ref_index % numcols);

			const unsigned opposite_line =
				lin.is_row ? (ref_index % numcols + numrows) : (ref_index / numcols);

			lines[opposite_line]->needs_line_solving = true;

			if (!performSingleCellRules(value, opposite_line, opposite_index))
				return false;
		}
		else if (grid[ref_index] != value)
		{
			return false;
		}
	}
	return true;
}

bool nonagram::performSingleCellRules(cell_state value, unsigned lin, unsigned idx)
{
	if (value == cell_state::filled)
	{
		for (auto& [length, positions] : lines[lin]->fills)
		{
			// If the filled space is immediately before or after
			// a possible fill, remove the possibility.

			std::erase_if(positions, [idx, length](unsigned start)
			              { return (start == idx + 1) || (start + length == idx); });

			if (positions.empty())
				return false;
		}
	}
	else
	{
		for (auto& [length, positions] : lines[lin]->fills)
		{
			// If the empty space is within the fill,
			// remove the possibility.

			std::erase_if(positions, [idx, length](unsigned start)
			              { return (start <= idx) && (idx < start + length); });

			if (positions.empty())
				return false;
		}
	}
	return true;
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
fill has no more candidates, returns false.
-------------------------------------------------*/

bool nonagram::removeIncompatible(line& lin)
{
	// Check each position for a space that has recently been marked
	for (unsigned i = 0; i < lin.grid.size(); ++i)
	{
		if (grid[lin.grid[i]] == cell_state::filled)
		{
			// These rules should be done regardless, as they may change over time.

			// Filled space cannot fall between two adjacent fills
			for (unsigned j = 0; j < lin.fills.size(); ++j)
			{
				// If the first fill comes after the filled space, or
				// If the first of the two fills cannot reach the filled
				// space, the second cannot start after it.

				if (j == 0 || lin.fills[j - 1].candidates.back() + lin.fills[j - 1].length < i)
				{
					// Delete all possibilites that start after the filled
					// space.
					while (i < lin.fills[j].candidates.back())
					{
						lin.fills[j].candidates.pop_back();
						if (lin.fills[j].candidates.empty())
							return false;
					}
				}

				// If the last fill ends before the filled space, or
				// If the second cannot reach it, the first cannot end
				// before it.

				if (j == lin.fills.size() - 1 || i < lin.fills[j + 1].candidates.front())
				{
					// Delete all possibilites that end before the filled
					// space.
					while (lin.fills[j].candidates.front() + lin.fills[j].length < i)
					{
						lin.fills[j].candidates.pop_front();
						if (lin.fills[j].candidates.empty())
							return false;
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
		const unsigned pos_after = lin.fills[i - 1].candidates.front() + lin.fills[i - 1].length;

		// This can be tested efficiently, since pos_after must be strictly
		// before the starting cell of the next fill, so remove candidates of
		// the next fill that start before or on the pos_after cell.
		while (pos_after >= lin.fills[i].candidates.front())
		{
			lin.fills[i].candidates.pop_front();
			if (lin.fills[i].candidates.empty())
				return false;
		}
	}

	// Push-back correcting. This loop goes backwards, since pushing a fill
	// backwards can affect fills before it.
	for (unsigned i = static_cast<unsigned>(lin.fills.size()) - 1; i > 0; --i)
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
			if (lin.fills[i - 1].candidates.empty())
				return false;
		}
	}
	return true;
}

/*--------------------------------------------------------------------
Marks items in the grid that are consistent with all possibilites in
a line. Important property: No cells marked will cause any fill
candidates within this line to be invalidated (in other words, there
is no need to call removeIncompatible after markConsistent is called).
--------------------------------------------------------------------*/

bool nonagram::markConsistent(line& lin)
{
	// Mark filled spaces

	// This rule does satisfy the mentioned property, however it isn't obvious.
	// First note that all marked cells are between the last position of the
	// first possible location, and the first position of the last possible
	// location. There will certainly not be a candidate from this fill that
	// gets removed, and there cannot be a candidate removed from an adjacent
	// fill, due to push back/forward correcting (it should be obvious from
	// this point that no other fills can be modified).
	for (auto& [length, positions] : lin.fills)
	{
		// Mark every cell from the last possible start position of the fill to
		// the first possible end position of the fill. This may not mark anything.
		if (!markInRange(lin, positions.back(), positions.front() + length, cell_state::filled))
			return false;
	}

	// Mark empty spaces

	// These rules obviously satisfy the stated property, as no candidates in
	// any of these touch the cells being marked, which is why they are being
	// marked as empty in the first place.

	// Mark every cell that is before all possible start positions of the first
	// fill as empty.

	if (!markInRange(lin, 0, lin.fills[0].candidates.front(), cell_state::empty))
		return false;

	// Mark every cell that is after all possible end positions of the last
	// fill as empty.

	unsigned lastoflast = lin.fills.back().candidates.back() + lin.fills.back().length;

	if (!markInRange(lin, lastoflast, lin.grid.size(), cell_state::empty))
		return false;

	// For each consecutive pair of fills, fill any gaps between
	// the last possibility of the first and the first possibility
	// of the second with empty space.
	for (unsigned i = 1; i < lin.fills.size(); ++i)
	{
		unsigned lastoffirst = lin.fills[i - 1].candidates.back() + lin.fills[i - 1].length;
		unsigned firstoflast = lin.fills[i].candidates.front();

		if (!markInRange(lin, lastoffirst, firstoflast, cell_state::empty))
			return false;
	}
	return true;
}

/*-------------------------------------------------------------
Calls removeIncompatible() and markConsistent(), in that order,
on each line. Returns false if any line is unsolvable.
-------------------------------------------------------------*/

bool nonagram::line_solve()
{
	// Index of the last line that was line solved in this method call,
	// starts as an unreachable value. It is assumed that at least one line
	// is marked for line solving.
	unsigned last_solved = static_cast<unsigned>(lines.size());
	while (true)
	{
		for (unsigned i = 0; i < lines.size(); ++i)
		{
			if (i == last_solved)
				return true;

			auto& lin = lines[i];

			if (!lin || !lin->needs_line_solving)
				continue;

			if (!removeIncompatible(*lin))
				return false;
			if (!markConsistent(*lin))
				return false;

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

	std::vector<unsigned> hintList;
	for (unsigned i = 0; i < CP.numrows; ++i)
	{
		getList(stream, hintList);

#ifdef CPUZZLE_DEBUG
		std::cout << "    Row " << i << ": ";
#endif

		CP.evaluateHintList(index_generator(CP.numcols * i, 1u, CP.numcols), hintList, i, true);

		hintList.clear();
	}

	for (unsigned i = 0; i < CP.numcols; ++i)
	{
		getList(stream, hintList);

#ifdef CPUZZLE_DEBUG
		std::cout << "    Column " << i << ": ";
#endif

		CP.evaluateHintList(index_generator(i, CP.numcols, CP.numrows), hintList, CP.numrows + i,
		                    false);

		hintList.clear();
	}

#ifdef CPUZZLE_DEBUG
	std::cout << "Successfully read from file.\n";
#endif

	return stream;
}

bool nonagram::solve()
{
#ifdef CPUZZLE_DEBUG
	std::cout << "Entering solve:\nPuzzle:\n" << *this << "Line solving:\n";
#endif

	if (!line_solve())
		return false;

	if (isComplete())
	{
#ifdef CPUZZLE_DEBUG
		std::cout << "Puzzle complete:\n" << *this;
#endif

		return true;
	}

#ifdef CPUZZLE_DEBUG
	std::cout << "\nUsing brute force, " << lines_to_solve
			  << " lines left"
				 " to solve:\n\n";
#endif

	// find position to brute force
	unsigned pos = 0;
	while (grid[pos] != cell_state::unknown)
		++pos;

	const unsigned rownum = pos / numcols, colnum = pos % numcols;

	// Mark the affected row and column, respectfully, as needing line solving.
	lines[rownum]->needs_line_solving = true;
	lines[numrows + colnum]->needs_line_solving = true;

	// For now, naively guess filled. This guess will be improved in the future.
	auto guess = cell_state::filled;

	nonagram copy(*this);

#ifdef CPUZZLE_DEBUG
	std::cout << "Guessing " << (guess == cell_state::filled ? "filled" : "empty")
			  << " at position " << pos << "(" << (pos / numrows) << "," << (pos % numrows)
			  << ")\n";
#endif

	copy.grid[pos] = guess;

	// Perform single cell rules on row, then column.
	if (copy.performSingleCellRules(guess, rownum, colnum) &&
	    copy.performSingleCellRules(guess, numrows + colnum, rownum))
	{
		if (copy.solve())
		{
			std::swap(copy, *this);
			return true;
		}
	}

	guess = (guess == cell_state::filled) ? cell_state::empty : cell_state::filled;

#ifdef CPUZZLE_DEBUG
	std::cout << "Guess at position " << pos << "(" << (pos / numrows) << "," << (pos % numrows)
			  << ") was incorrect, trying " << (guess == cell_state::filled ? "filled" : "empty")
			  << " instead\n";
#endif

	// No need to use the copy anymore, if this solves the puzzle the solution
	// is already in place, if not, the puzzle is "junk" anyways.
	grid[pos] = guess;

	// Perform single cell rules on row, then column.
	if (!performSingleCellRules(guess, rownum, colnum))
		return false;
	if (!performSingleCellRules(guess, colnum + numrows, rownum))
		return false;

	// Since this is the last attempt, let the result escalate.
	return solve();
}

bool nonagram::isComplete() const { return lines_to_solve == 0; }

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
