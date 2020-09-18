#ifndef BMP_MAKER_H
#define BMP_MAKER_H

#include <fstream>
#include <vector>

/*-----------------------------------------
A color_24 is a 24 bit color. Contains BGR.
-----------------------------------------*/

#define DEFAULT_B 255
#define DEFAULT_G 255
#define DEFAULT_R 255

struct color_24
{
	char B, G, R;
	
	color_24() : B(DEFAULT_B), G(DEFAULT_G), R(DEFAULT_R) {}
	
	color_24(char b, char g, char r) : B(b), G(g), R(r) {}
	
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
	private:
	
	int width;
	int height;
	
	// Row 0 is the bottom row, column 0 is the left
	std::vector<std::vector<color_24>> grid;
	
	static void writeLittleEndian(int e, std::ostream& out, int numBytes);
	
	public:
	
	// Creates a bitmap of specified size (default white)
	BMP_24(int hgt, int wid);
	
	// Creates a bitmap of specified size and default color
	BMP_24(int hgt, int wid, color_24 def);
	
	// Loads a bitmap from a file
	// BMP_24(const char* file);

	color_24& position(int x, int y);
	
	friend std::ostream& operator<<(std::ostream& out, const BMP_24& bmp);
	
	void write(const char* file) const;
};

#endif
