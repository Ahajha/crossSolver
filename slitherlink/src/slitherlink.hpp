#pragma once

#include <array>
#include <cstdint>
#include <vector>

class SlitherlinkPuzzle {
public:
  enum class Line { unknown, filled, empty };
  struct Cell {
    std::int8_t n_adjacent;
    constexpr static std::int8_t unknown = -1;
  };
  // Unknown if the corners need any data currently
  struct Corner {};

  struct LineCoord {
    std::uint8_t row, col;
  };
  struct CellCoord {
    std::uint8_t row, col;
  };
  struct CornerCoord {
    std::uint8_t row, col;
  };

  auto corner_coords(const CellCoord coord) -> std::array<CornerCoord, 4> {
    return {{
        {coord.row, coord.col},
        {coord.row, static_cast<std::uint8_t>(coord.col + 1)},
        {coord.row, coord.col},
        {coord.row, coord.col},
    }};
  }

  auto corner(const CornerCoord coord) -> Corner & { return corners[coord.]; }

private:
  // clang-format off
  // How should the data be represented? We have a few options:
  // 1: Store the corners, along with which directions, if any, lines are
  // protruding from.
  //    (there would only ever be 0 or 2, which might help with storage)
  // 2: Store the line segments, grouped by rows and columns.
  // 3: Store the squares, with line segments stored seperately.

  // I think 3 is the best starting point:
  // Store the squares
  // Store the corners - unsure if there actually needs to be any data stored,
  // perhaps other than number of adjacent segments.
  // Store the segments seperately. There should be some way to determine which
  // segments go to which squares, probably a formula on the index. Similarly,
  // there should be some formula to get the adjacent segments and squares based
  // on index.

  // Essentially: Store squares, corners, and edges. Each should have some sort
  // of computation to let each reference other in a meaningful way.

  // Brainstorming:

  // Squares can be indexed as so:
  // 0 1 2
  // 3 4 5
  // 6 7 8
  
  // Corners can be stored in a similar order
  // 0  1  2  3
  // 4  5  6  7
  // 8  9  10 11
  // 12 13 14 15

  // To go from the square at (row, col) to its four corners:
  // (row, col), (row + 1, col), (row, col + 1), (row + 1, col + 1)
  // Except that this uses the corner grid's dimensions, which is n_rows + 1 by n_cols + 1.
  // So for (row, col), the grid square is at row * n_cols + col
  // Top left corner is row * (n_cols + 1) + col
  // Top right corner is row * (n_cols + 1) + col + 1
  // Bottom left corner is (row + 1) * (n_cols + 1) + col
  // Bottom right corner is (row + 1) * (n_cols + 1) + col + 1

  // For now, it will be easier to keep lines split into horizontal and vertical.
  // Similar formation, the structure is just a grid with the lines.

  // clang-format on

  std::size_t n_rows, n_cols;
  std::vector<Cell> cells;
  std::vector<Corner> corners;
  std::vector<Line> horizontal_lines, vertical_lines;
};
