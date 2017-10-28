#include "global.h"


// good ol' constructor
Scanline::Scanline(uint width, uchar components) {

	w = width;
	c = components;

	for (uint it = 0; it < width; it++)
		data.push_back(Pixel(c));

}

// boring getters
ulong Scanline::pixels() const { return w; }
uint Scanline::width() const { return w; }
uint Scanline::height() const { return 1; }
uchar Scanline::channels() const { return c; }

// not so boring getter
float Scanline::max() const {

	// width = 0? That ain't good
	if (w == 0)
		return numeric_limits<float>::signaling_NaN();


	// grind through it all
	float max = data[0].max();
	for (uint it = 1; it < w; it++)
		if (data[it].max() > max)
			max = data[it].max();

	return max;	// don't update maxAll!

}

// make this easy on programmers
Pixel& Scanline::operator[](size_t index) {

	if (index >= data.size())
		throw new out_of_range("Scanline: Pixel out of range");
	else
		return data[index];

}

const Pixel& Scanline::operator[](size_t index) const {

	if (index >= data.size())
		throw new out_of_range("Scanline: Pixel out of range");
	else
		return data[index];

}
