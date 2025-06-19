#pragma once
#include <tuple>

class Grid3D {
public:
	int x = 0;
	int y = 0;
	int z = 0;

	Grid3D() = default;
	Grid3D(int x, int y, int z): x(x), y(y), z(z) {}

	bool operator<(const Grid3D& other) const {
		return std::tie(z, y, x) < std::tie(other.z, other.y, other.x);
	}

	bool operator==(const Grid3D& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
};
