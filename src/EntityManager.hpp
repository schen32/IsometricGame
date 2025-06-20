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

	void removeDeadEntities(EntityVec& vec)
	{
		vec.erase(
			std::remove_if
			(
				vec.begin(),
				vec.end(),
				[](Entity entity)
				{
					return !entity.isActive();
				}
			),
			vec.end()
		);
	}

	EntityManager() = default;

	void update()
	{
		for (auto& entity : m_entitiesToAdd)
		{
			m_entities.push_back(entity);
			auto& entityTag = entity.tag();
			m_entityMap[entityTag].push_back(entity);
		}
		m_entitiesToAdd.clear();

		removeDeadEntities(m_entities);
		for (auto& [tag, entityVec] : m_entityMap)
		{
			removeDeadEntities(entityVec);
		}
	}

	Entity addEntity(const std::string& tag, const std::string& name)
	{
		auto entity = MemoryPool::Instance().addEntity(tag, name);
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