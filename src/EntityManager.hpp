#pragma once

#include "Entity.hpp"
#include "MemoryPool.hpp"
#include <vector>
#include <unordered_map>

using EntityVec = std::vector<Entity>;

class EntityManager
{
public:
	EntityVec m_entities;
	EntityVec m_entitiesToAdd;
	std::unordered_map<std::string, EntityVec> m_entityMap;

	void removeDeadEntities(MemoryPool& pool, EntityVec& vec)
	{
		vec.erase(
			std::remove_if
			(
				vec.begin(),
				vec.end(),
				[&pool](Entity entity)
				{
					return !entity.isActive(pool);
				}
			),
			vec.end()
		);
	}

	EntityManager() = default;

	void update(MemoryPool& pool)
	{
		for (auto& entity : m_entitiesToAdd)
		{
			m_entities.push_back(entity);
			auto& entityTag = entity.tag(pool);
			m_entityMap[entityTag].push_back(entity);
		}
		m_entitiesToAdd.clear();

		if (!pool.m_entityRemoved) return;
		pool.m_entityRemoved = false;

		removeDeadEntities(pool, m_entities);
		for (auto& [tag, entityVec] : m_entityMap)
		{
			removeDeadEntities(pool, entityVec);
		}
	}

	Entity addEntity(MemoryPool& pool, const std::string& tag, const std::string& name)
	{
		auto entity = pool.addEntity(tag, name);
		m_entitiesToAdd.push_back(entity);
		return entity;
	}

	EntityVec& getEntities()
	{
		return m_entities;
	}

	EntityVec& getEntities(const std::string& tag)
	{
		if (m_entityMap.find(tag) == m_entityMap.end())
			m_entityMap[tag] = EntityVec();
		return m_entityMap[tag];
	}

	std::unordered_map<std::string, EntityVec>& getEntityMap()
	{
		return m_entityMap;
	}
};