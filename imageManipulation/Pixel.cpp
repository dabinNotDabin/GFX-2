#include "global.h"

// good ol' constructor
Pixel::Pixel(uchar components) {

	c.reserve(components);

	maxAll = numeric_limits<float>::signaling_NaN();

	for (uchar it = 0; it < components; it++)
		c.push_back(numeric_limits<float>::signaling_NaN());

}

// boring getters
ulong Pixel::pixels() const { return 1; }
uint Pixel::width() const { return 1; }
uint Pixel::height() const { return 1; }
uchar Pixel::channels() const { return c.size(); }
float Pixel::r() const { return get(0); }
float Pixel::g() const { return get(1); }
float Pixel::b() const { return get(2); }
bool Pixel::r(const float& v) { return set(0, v); }
bool Pixel::g(const float& v) { return set(1, v); }
bool Pixel::b(const float& v) { return set(2, v); }

// now boring getter
float Pixel::max() const { return maxAll; }

// less boring getter/setters
float Pixel::get(const uchar i) const {

	if (i >= c.size())
		throw new out_of_range("Pixel: Requested non-existant component");
	else
		return c[i];

}

bool Pixel::set(const uchar i, const float& v) {

	// the first part is easy
	if ((i >= c.size()) || isnan(v))
		return false;
	else {

		c[i] = v;

		// did we break our maximum? 
		if (isnan(maxAll) || (v > maxAll))

			// then update
			maxAll = v;
	}

	return true;
}
