#pragma once

#include "Entity.hpp"
#include <vector>
#include <unordered_map>

using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
	EntityVec m_entities;
	EntityVec m_entitiesToAdd;
	std::unordered_map<std::string, EntityVec> m_entityMap;
	size_t m_totalEntities = 0;

	void removeDeadEntities(EntityVec& vec)
	{
		vec.erase(
			std::remove_if
			(
				vec.begin(),
				vec.end(),
				[](const std::shared_ptr<Entity> entity)
				{
					return !entity->isActive();
				}
			),
			vec.end()
		);
	}

public:
	EntityManager() = default;

	void update()
	{
		for (auto& entity : m_entitiesToAdd)
		{
			m_entities.push_back(entity);
			m_entityMap[entity->tag()].push_back(entity);
		}
		m_entitiesToAdd.clear();

		removeDeadEntities(m_entities);
		for (auto& [tag, entityVec] : m_entityMap)
		{
			removeDeadEntities(entityVec);
		}
	}

	std::shared_ptr<Entity> addEntity(const std::string& tag, const std::string& name)
	{
		auto entity = std::shared_ptr<Entity>(new Entity(tag, name, m_totalEntities++));
		// auto entity = std::make_shared<Entity>(tag, m_totalEntities++);
		m_entitiesToAdd.push_back(entity);
		return entity;
	}

	const EntityVec& getEntities()
	{
		return m_entities;
	}

	const EntityVec& getEntities(const std::string& tag)
	{
		if (m_entityMap.find(tag) == m_entityMap.end())
			m_entityMap[tag] = EntityVec();
		return m_entityMap[tag];
	}

	const std::unordered_map<std::string, EntityVec>& getEntityMap()
	{
		return m_entityMap;
	}
};