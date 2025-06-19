#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "Components.hpp"

class Utils
{
public:
	Utils() = default;

	bool static IsInside(const Vec2f& pos, std::shared_ptr<Entity> entity)
	{
		auto eTransform = entity->get<CTransform>();
		auto ePosition = eTransform.pos;
		auto eScale = eTransform.scale;
		auto eSize = entity->get<CAnimation>().animation.m_size;
		eSize.x *= eScale.x;
		eSize.y *= eScale.y;

		if (ePosition.x - eSize.x / 2 <= pos.x && pos.x <= ePosition.x + eSize.x / 2 &&
			ePosition.y - eSize.y / 2 <= pos.y && pos.y <= ePosition.y + eSize.y / 2)
		{
			return true;
		}
		return false;
	}
};