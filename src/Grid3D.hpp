#pragma once
#include <tuple>

class Grid3D {
public:
	int x = 0;
	int y = 0;
	int z = 0;

	Grid3D() = default;
	Grid3D(int ix, int iy, int iz): x(ix), y(iy), z(iz) {}

	bool operator<(const Grid3D& other) const {
		return std::tie(z, y, x) < std::tie(other.z, other.y, other.x);
	}

	bool operator==(const Grid3D& other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	Grid3D operator+(const Grid3D& other) const {
		return Grid3D(x + other.x, y + other.y, z + other.z);
	}

	Grid3D operator/(int other) const {
		return Grid3D(x / other, y / other, z / other);
	}

	void operator +=(const Grid3D& other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

	int volume() const {
		return x * y * z;
	}
};
