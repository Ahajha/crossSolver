#pragma once

#include <ostream>
#include <vector>
#include <cstdint>

/*-----------------------------------------
A color_24 is a 24 bit color. Contains BGR.
-----------------------------------------*/

struct color_24
{
	std::uint8_t B, G, R;
	
	friend std::ostream& operator<<(std::ostream& out, const color_24& col);
};

/*-----------------------------------------
Constains various useful predefined colors.
-----------------------------------------*/

namespace color_24_consts
{
	constexpr static color_24 white (255,255,255);
	constexpr static color_24 light_gray (64,64,64);
	constexpr static color_24 gray (127,127,127);
	constexpr static color_24 dark_gray (192,192,192);
	constexpr static color_24 black (0,0,0);

	constexpr static color_24 blue (255,0,0);
	constexpr static color_24 green (0,255,0);
	constexpr static color_24 red (0,0,255);

	constexpr static color_24 yellow (255,255,0);
	constexpr static color_24 pink (255,192,203);
	constexpr static color_24 light_blue (173,216,230);
	constexpr static color_24 light_green (144,238,144);
	constexpr static color_24 orange (255,165,0);
	constexpr static color_24 light_brown (181,101,29);
	constexpr static color_24 magenta (255,0,255);
}

/*----------------------------------------------------
BMP_24 can be used to make or modify 24-bit BMP files.
----------------------------------------------------*/

class BMP_24
{
	unsigned width, height;
	
	// Row 0 is the bottom row, column 0 is the left
	std::vector<color_24> grid;
	
	static void writeLittleEndian(unsigned e, std::ostream& out, unsigned numBytes);
	
	public:
	
	// Creates a bitmap of specified size and base color.
	// Base color defaults to white.
	BMP_24(unsigned hgt, unsigned wid, color_24 def = color_24_consts::white);
	
	color_24& operator()(unsigned x, unsigned y);
	color_24  operator()(unsigned x, unsigned y) const;
	
	friend std::ostream& operator<<(std::ostream& out, const BMP_24& bmp);
	
	void write(const std::string& filename) const;
};
