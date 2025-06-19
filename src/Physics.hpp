#pragma once

#include "Entity.hpp"
#include "Components.hpp"

struct Intersect
{
	bool intersected = false;
	Vec2f point;
};

class Physics
{
public:
	Physics() = default;

	Vec2f static GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
	{
		if (!(a->has<CBoundingBox>() && b->has<CBoundingBox>()))
		{
			return Vec2f(0.0f, 0.0f);
		}

		auto& aTransform = a->get<CTransform>();
		auto& bTransform = b->get<CTransform>();

		auto delta = Vec2f(abs(aTransform.pos.x - bTransform.pos.x),
			abs(aTransform.pos.y - bTransform.pos.y));
		
		auto& aBB = a->get<CBoundingBox>();
		auto& bBB = b->get<CBoundingBox>();

		float xOverlap = aBB.halfSize.x + bBB.halfSize.x - delta.x;
		float yOverlap = aBB.halfSize.y + bBB.halfSize.y - delta.y;
		return Vec2f(xOverlap, yOverlap);
	}

	Vec2f static GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
	{
		if (!(a->has<CBoundingBox>() && b->has<CBoundingBox>()))
		{
			return Vec2f(0.0f, 0.0f);
		}

		auto& aTransform = a->get<CTransform>();
		auto& bTransform = b->get<CTransform>();

		auto delta = Vec2f(abs(aTransform.prevPos.x - bTransform.prevPos.x),
			abs(aTransform.prevPos.y - bTransform.prevPos.y));

		auto& aBB = a->get<CBoundingBox>();
		auto& bBB = b->get<CBoundingBox>();

		float xOverlap = aBB.halfSize.x + bBB.halfSize.x - delta.x;
		float yOverlap = aBB.halfSize.y + bBB.halfSize.y - delta.y;
		return Vec2f(xOverlap, yOverlap);
	}
};