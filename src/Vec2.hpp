#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

template <typename T>
class Vec2
{
public:
	T x = 0;
	T y = 0;

	Vec2() = default;

	Vec2(T xin, T yin)
		: x(xin), y(yin)
	{ }

	Vec2(const sf::Vector2<T>& vec)
		: x(vec.x), y(vec.y)
	{ }

	operator sf::Vector2<T>() const
	{
		return sf::Vector2<T>(x, y);
	}

	// Constructor for any SFML vector type
	template <typename U>
	Vec2(const sf::Vector2<U>& vec)
		: x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y))
	{
	}

	// Conversion operator to any SFML vector type
	template <typename U>
	operator sf::Vector2<U>() const
	{
		return sf::Vector2<U>(static_cast<U>(x), static_cast<U>(y));
	}

	Vec2 operator + (const Vec2& rhs) const
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}

	Vec2 operator - (const Vec2& rhs) const
	{
		return Vec2(x - rhs.x, y - rhs.y);
	}

	Vec2 operator / (const T val) const
	{
		return Vec2(x / val, y / val);
	}

	Vec2 operator * (const T val) const
	{
		return Vec2(x * val, y * val);
	}

	bool operator == (const Vec2& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}

	bool operator != (const Vec2& rhs) const
	{
		return !(x == rhs.x && y == rhs.y);
	}

	void operator += (const Vec2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
	}

	void operator -= (const Vec2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
	}

	void operator *= (const T val)
	{
		x *= val;
		y *= val;
	}

	void operator /= (const T val)
	{
		x /= val;
		y /= val;
	}

	float distTo(const Vec2& rhs) const
	{
		float dx = static_cast<float>(x - rhs.x);
		float dy = static_cast<float>(y - rhs.y);
		return std::sqrt(dx * dx + dy * dy);
	}

	float distToSquared(const Vec2& rhs) const
	{
		float dx = static_cast<float>(x - rhs.x);
		float dy = static_cast<float>(y - rhs.y);
		return dx * dx + dy * dy;
	}

	float length() const
	{
		return std::sqrt(static_cast<float>(x * x + y * y));
	}

	float lengthSquared() const
	{
		return static_cast<float>(x * x + y * y);
	}
	
	Vec2 normalize() const
	{
		float len = length();
		if (len > 0)
		{
			return Vec2(x / len, y / len);
		}
		return Vec2(x, y);
	}
};

using Vec2f = Vec2<float>;