#pragma once

#include <fstream>
#include <vector>

/*-----------------------------------------
A color_24 is a 24 bit color. Contains BGR.
-----------------------------------------*/

struct color_24
{
	constexpr static uint8_t
		DEFAULT_B = 255, DEFAULT_G = 255, DEFAULT_R = 255;
	uint8_t B, G, R;
	
	color_24() : B(DEFAULT_B), G(DEFAULT_G), R(DEFAULT_R) {}
	
	color_24(uint8_t b, uint8_t g, uint8_t r) : B(b), G(g), R(r) {}
	
	// Constants
	static const color_24 white;
	static const color_24 light_gray;
	static const color_24 gray;
	static const color_24 dark_gray;
	static const color_24 black;
	
	static const color_24 red;
	static const color_24 blue;
	static const color_24 green;
	
	static const color_24 yellow;
	static const color_24 pink;
	static const color_24 light_blue;
	static const color_24 light_green;
	static const color_24 orange;
	static const color_24 light_brown;
	static const color_24 magenta;
	
	friend std::ostream& operator<<(std::ostream& out, const color_24& col);
};

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
	BMP_24(unsigned hgt, unsigned wid, color_24 def = color_24{});
	
	color_24& operator()(unsigned x, unsigned y);
	color_24  operator()(unsigned x, unsigned y) const;
	
	friend std::ostream& operator<<(std::ostream& out, const BMP_24& bmp);
	
	void write(const std::string& filename) const;
};
