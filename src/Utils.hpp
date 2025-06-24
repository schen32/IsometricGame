#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "Components.hpp"

class Utils
{
public:
	Utils() = default;

	static sf::FloatRect visibleArea(const sf::View& cameraView)
	{
		const float padding = 128.0f;
		sf::Vector2f viewCenter = cameraView.getCenter();
		sf::Vector2f viewSize = cameraView.getSize() + sf::Vector2f(padding, padding);
		sf::FloatRect visibleArea(viewCenter - viewSize / 2.f, viewSize);
		return visibleArea;
	}

	static bool isVisible(const CTransform& eTransform, const sf::FloatRect& visibleArea)
	{
		auto& pos = eTransform.pos;
		return visibleArea.contains(pos);
	}

	bool static isInside(const Vec2f& point, const CTransform& eTransform, const CAnimation& eAnimation)
	{
		const Vec2f& center = eTransform.pos;
		const Vec2f& scale = eTransform.scale;

		Vec2f size = eAnimation.animation.m_size;
		size.x *= scale.x;
		size.y *= scale.y;

		const float left = center.x - size.x / 2.0f;
		const float right = center.x + size.x / 2.0f;
		const float top = center.y - size.y / 2.0f;
		const float bottom = center.y + size.y / 2.0f;

		return (point.x >= left && point.x <= right &&
			point.y >= top && point.y <= bottom);
	}

	bool static isInside(const Vec2f& point, const CTransform& eTransform, const Vec2f& eSize)
	{
		const Vec2f& center = eTransform.pos;
		const Vec2f& scale = eTransform.scale;

		Vec2f size = eSize;
		size.x *= scale.x;
		size.y *= scale.y;

		const float left = center.x - size.x / 2.0f;
		const float right = center.x + size.x / 2.0f;
		const float top = center.y - size.y / 2.0f;
		const float bottom = center.y + size.y / 2.0f;

		return (point.x >= left && point.x <= right &&
			point.y >= top && point.y <= bottom);
	}
};