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
		return std::tie(z, y, x) < std::tie(other.z, other.y, other.x);
	}

	bool operator==(const Grid3D& other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	Grid3D operator+(const Grid3D& other) const {
		return Grid3D(x + other.x, y + other.y, z + other.z);
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

	struct Grid3DHash {
		size_t operator()(const Grid3D& g) const {
			return std::hash<float>()(g.x) ^ (std::hash<float>()(g.y) << 1) ^ (std::hash<float>()(g.z) << 2);
		}
	};
};
