// Author:        Alex Trotta
// Start Date:    3/20/18
// Last Modified: 12/19/18
// File Name:     bmpMaker.cpp

#include <iostream>
#include <fstream>
#include <vector>
#include "bmpMaker.h"

const color_24 color_24::white (255,255,255);
const color_24 color_24::light_gray (64,64,64);
const color_24 color_24::gray (127,127,127);
const color_24 color_24::dark_gray (192,192,192);
const color_24 color_24::black (0,0,0);

const color_24 color_24::red (0,0,255);
const color_24 color_24::blue (255,0,0);
const color_24 color_24::green (0,255,0);

const color_24 color_24::yellow (255,255,0);
const color_24 color_24::pink (255,192,203);
const color_24 color_24::light_blue (173,216,230);
const color_24 color_24::light_green (144,238,144);
const color_24 color_24::orange (255,165,0);
const color_24 color_24::light_brown (181,101,29);
const color_24 color_24::magenta (255,0,255);

BMP_24::BMP_24 (int hgt, int wid) : width(wid), height(hgt)
{
	grid.resize(height);
	for (int i = 0; i < hgt; i++)
	{
		grid[i].resize(width);
	}
}

BMP_24::BMP_24(int hgt, int wid, color_24 def) : BMP_24(hgt,wid)
{
	for (int i = 0; i < height; i++)
	{
		std::fill(grid[i].begin(), grid[i].end(), def);
	}
}

void BMP_24::writeLittleEndian(int e, std::ostream& out, int numBytes)
{
	int x = e;
	for (int i = 0; i < numBytes; i++)
	{
		out << (char)(x % 256);
		x /= 256;
	}
}

// X is row #
color_24& BMP_24::position(int x, int y)
{
	return grid[x][y];
}

void BMP_24::write(const char* file) const
{
	std::ofstream out(file, std::ios_base::binary);
	
	out << *this;
	
	out.close();
}

std::ostream& operator<<(std::ostream& out, const BMP_24& bmp)
{
	// ID field ("BM") // Must be used
	out << 'B';
	out << 'M';
	
	// Size of the bitmap data, in bytes, including padding. Used twice
	int bitmapSize = bmp.height*((bmp.width*3)/4)*4 + ((bmp.width%4 != 0) * 4);

	// Size of the file // Used, only pixel size and amount will change this
	BMP_24::writeLittleEndian(54 + bitmapSize, out, 4);
	
	// Unused (x2)
	BMP_24::writeLittleEndian(0, out, 4);
	
	// Start position of pixel array // Used, no features will change this
	BMP_24::writeLittleEndian(54, out, 4);
	
	/* DIB Header */
	
	// Number of bytes in the header // Used, no features will change this
	BMP_24::writeLittleEndian(40, out, 4);
	
	// Width of the bitmap in pixels (left to right) // Used
	BMP_24::writeLittleEndian(bmp.width, out, 4);
	
	// Height of the bitmap in pixels (bottom to top) // Used
	BMP_24::writeLittleEndian(bmp.height, out, 4);
	
	// Number of color planes used // No feature for this
	out << (char)1;
	out << (char)0;
	
	// Number of bits per pixel // No feature for this
	out << (char)24;
	out << (char)0;

	// Compression type // No feature for this
	BMP_24::writeLittleEndian(0, out, 4);
	
	// Size of raw bitmap data in bytes (including padding) // Must be used
	BMP_24::writeLittleEndian(bitmapSize, out, 4);
	
	// Print resolution:
	
	// p/m (horizontal) // No feature for this
	BMP_24::writeLittleEndian(2835, out, 4);
	
	// p/m (vertical) // No feature for this
	BMP_24::writeLittleEndian(2835, out, 4);
	
	// Number of colors in the palette // No feature for this
	BMP_24::writeLittleEndian(0, out, 4);
	
	// Important colors (0 means all) // No feature for this
	BMP_24::writeLittleEndian(0, out, 4);
	
	/* Pixel Array */
	
	int padding = (bmp.width % 4 == 0) ? 0 : (bmp.width % 4);
	
	// From the bottom left, going row by row.
	for (int i = 0; i < bmp.height; i++)
	{
		for (int j = 0; j < bmp.width; j++)
		{
			out << bmp.grid[i][j];
		}
		BMP_24::writeLittleEndian(0, out, padding);
	}
	
	return out;
}

std::ostream& operator<<(std::ostream& out, const color_24& col)
{
	return (out << col.B << col.G << col.R);
}
