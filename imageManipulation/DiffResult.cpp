#include "global.h"

// allow these to be sorted
bool DiffResult::operator<(const DR& other) const {

	// y > x in all cases

	if (y != other.y)
		return y < other.y;
	else
		return x < other.x;

}
