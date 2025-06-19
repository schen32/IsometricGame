#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Grid3D.hpp"

class Utils
{
public:
	Utils() = default;

	bool static isInsideTopFace(const Vec2f& pos, std::shared_ptr<Entity> entity)
	{
		auto eTransform = entity->get<CTransform>();
		Vec2f spriteCenter = eTransform.pos; // center of entire sprite
		Vec2f scale = eTransform.scale;
		Vec2f size = entity->get<CAnimation>().animation.m_size;

		// Apply scale
		size.x *= scale.x;
		size.y *= scale.y;

		// Calculate center of the top diamond face (top half of sprite)
		Vec2f topFaceCenter = spriteCenter - Vec2f(0, size.y / 4.0f);  // Move up by 1/4 height

		// Diamond bounds
		float halfWidth = size.x / 2.0f;
		float halfHeight = size.y / 4.0f; // Because top face is only top half

		// Check point-in-diamond condition
		float dx = std::abs(pos.x - topFaceCenter.x);
		float dy = std::abs(pos.y - topFaceCenter.y);

		return (dx / halfWidth + dy / halfHeight) <= 1.0f;
	}


	Vec2f static gridToIsometric(Grid3D gridPos, std::shared_ptr<Entity> entity)
	{
		auto& eAnimation = entity->get<CAnimation>();
		Vec2f eSize = eAnimation.animation.m_size;

		Vec2f i = Vec2f(eSize.x / 2, 0.5f * eSize.y / 2);
		Vec2f j = Vec2f(-eSize.x / 2, 0.5f * eSize.y / 2);

		return (i * gridPos.x + j * gridPos.y) - Vec2f(0, gridPos.z * eSize.y / 2);
	}

	Vec2f static isometricToGrid(float isoX, float isoY, int z, Vec2f gridCellSize)
	{
		Vec2f i = Vec2f(gridCellSize.x / 2, 0.5f * gridCellSize.y / 2);
		Vec2f j = Vec2f(-gridCellSize.x / 2, 0.5f * gridCellSize.y / 2);

		// Apply inverse Z offset
		isoY += z * (gridCellSize.y / 2);

		float a = i.x, b = j.x;
		float c = i.y, d = j.y;

		float det = a * d - b * c;
		if (det == 0.0f) {
			std::cerr << "Invalid basis vectors: determinant is zero!\n";
			return Vec2f(0.f, 0.f);
		}

		float invDet = 1.0f / det;

		float gridX = invDet * (d * isoX - b * isoY);
		float gridY = invDet * (-c * isoX + a * isoY);

		return Vec2f(gridX, gridY);
	}
};