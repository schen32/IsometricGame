#pragma once
#include <tuple>
#include <sstream>
#include <string>
#include <cmath>

class Grid3D {
public:
	float x = 0;
	float y = 0;
	float z = 0;

	Grid3D() = default;
	Grid3D(float ix, float iy, float iz): x(ix), y(iy), z(iz) {}
	
	Grid3D floor() const {
		return Grid3D(std::floor(x), std::floor(y), std::floor(z));
	}

	std::string toString() const {
		std::ostringstream oss;
		oss << "(" << x << ", " << y << ", " << z << ")";
		return oss.str();
	}

	float distTo(const Grid3D& other) const {
		return std::sqrt(distToSquared(other));
	}
	
	float distToSquared(const Grid3D& other) const {
		float dx = x - other.x;
		float dy = y - other.y;
		float dz = z - other.z;
		return dx * dx + dy * dy + dz * dz;
	}

	bool operator<(const Grid3D& other) const {
		return std::tie(x, y, z) < std::tie(other.x, other.y, other.z);
	}

	bool operator==(const Grid3D& other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	Grid3D operator+(const Grid3D& other) const {
		return Grid3D(x + other.x, y + other.y, z + other.z);
	}

	Grid3D operator-(const Grid3D& other) const {
		return Grid3D(x - other.x, y - other.y, z - other.z);
	}

	Grid3D operator/(float other) const {
		return Grid3D(x / other, y / other, z / other);
	}

	void operator +=(const Grid3D& other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

	float volume() const {
		return x * y * z;
	}
};
