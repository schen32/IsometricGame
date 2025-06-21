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

	Vec2f static GetOverlap(const CBoundingBox& aBB, const CTransform& aTrans,
		const CBoundingBox& bBB, const CTransform& bTrans)
	{
		auto delta = Vec2f(abs(aTrans.pos.x - bTrans.pos.x),
			abs(aTrans.pos.y - bTrans.pos.y));
		
		float xOverlap = aBB.halfSize.x + bBB.halfSize.x - delta.x;
		float yOverlap = aBB.halfSize.y + bBB.halfSize.y - delta.y;
		return Vec2f(xOverlap, yOverlap);
	}

	Vec2f static GetPreviousOverlap(const CBoundingBox& aBB, const CTransform& aTrans,
		const CBoundingBox& bBB, const CTransform& bTrans)
	{
		auto delta = Vec2f(abs(aTrans.prevPos.x - bTrans.prevPos.x),
			abs(aTrans.prevPos.y - bTrans.prevPos.y));

		float xOverlap = aBB.halfSize.x + bBB.halfSize.x - delta.x;
		float yOverlap = aBB.halfSize.y + bBB.halfSize.y - delta.y;
		return Vec2f(xOverlap, yOverlap);
	}
};