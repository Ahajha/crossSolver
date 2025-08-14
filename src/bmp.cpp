#include "bmp.hpp"

#include <fstream>

BMP_24::BMP_24(unsigned h, unsigned w, color_24 def) : width(w), height(h), grid(w * h, def) {}

void BMP_24::writeLittleEndian(unsigned e, std::ostream& out, unsigned numBytes)
{
	unsigned x = e;
	for (unsigned i = 0; i < numBytes; ++i)
	{
		out << (char)(x % 256);
		x /= 256;
	}
}

// X is row #
color_24 BMP_24::operator()(unsigned x, unsigned y) const { return grid[x * width + y]; }

color_24& BMP_24::operator()(unsigned x, unsigned y) { return grid[x * width + y]; }

void BMP_24::write(const std::string& filename) const
{
	std::ofstream(filename, std::ios_base::binary) << *this;
}

std::ostream& operator<<(std::ostream& out, const BMP_24& bmp)
{
	// ID field ("BM") // Must be used
	out << "BM";

	// Size of the bitmap data, in bytes, including padding. Used twice
	const unsigned bitmapSize = bmp.height * ((bmp.width * 3) / 4) * 4 + ((bmp.width % 4 != 0) * 4);

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
	out << (char)1 << (char)0;

	// Number of bits per pixel // No feature for this
	out << (char)24 << (char)0;

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

	const unsigned padding = bmp.width % 4;

	// From the bottom left, going row by row.
	unsigned pos = 0;
	for (unsigned i = 0; i < bmp.height; ++i)
	{
		for (unsigned j = 0; j < bmp.width; ++j)
		{
			out << bmp.grid[pos++];
		}
		BMP_24::writeLittleEndian(0, out, padding);
	}

	return out;
}

std::ostream& operator<<(std::ostream& out, const color_24& col)
{
	return (out << col.B << col.G << col.R);
}
