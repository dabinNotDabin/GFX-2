#include "global.h"


// good ol' constructor
Image::Image(uint width, uint height, uchar components) {

	w = width;
	h = height;
	c = components;

	for (uint it = 0; it < height; it++)
		data.push_back(Scanline(w, c));

}

// boring getters
ulong Image::pixels() const { return w * h; }
uint Image::width() const { return w; }
uint Image::height() const { return h; }
uchar Image::channels() const { return c; }

// not so boring getter
float Image::max() const {

	// height = 0? That ain't good
	if (h == 0)
		return numeric_limits<float>::signaling_NaN();

	// otherwise, grind away
	float max = data[0].max();
	for (uint it = 1; it < h; it++)
		if (data[it].max() > max)
			max = data[it].max();

	return max;

}

// handle accesses in a programmer-friendly way
Scanline& Image::operator[](size_t index) {

	if (index >= data.size())
		throw new out_of_range("Image: Scanline out of range");
	else
		return data[index];

}

const Scanline& Image::operator[](size_t index) const {

	if (index >= data.size())
		throw new out_of_range("Image: Scanline out of range");
	else
		return data[index];

}
