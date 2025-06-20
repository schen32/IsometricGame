#pragma once

#include <tuple>
#include <vector>
#include "Components.hpp"

using ComponentVectorTuple = std::tuple<
	std::vector<CTransform>,
	std::vector<CGridPosition>,
	std::vector<CInput>,
	std::vector<CBoundingBox>,
	std::vector<CAnimation>,
	std::vector<CState>,
	std::vector<CHealth>,
	std::vector<CDamage>
>;

class Entity;

class MemoryPool
{
public:
	size_t						m_numEntities;
	size_t						m_maxEntities;
	ComponentVectorTuple		m_pool;
	std::vector<std::string>	m_tags;
	std::vector<std::string>	m_names;
	std::vector<bool>			m_active;

	MemoryPool(size_t maxEntities) : m_numEntities(0), m_maxEntities(maxEntities)
	{
		std::apply([&](auto&... componentVecs) {
			(..., componentVecs.resize(maxEntities));
			}, m_pool);

		m_tags.resize(maxEntities);
		m_names.resize(maxEntities);
		m_active.resize(maxEntities);
	}
	static MemoryPool& Instance()
	{
		const static size_t MAX_ENTITIES = 512 * 512;
		static MemoryPool pool(MAX_ENTITIES);
		return pool;
	}

	size_t getNextEntityIndex()
	{
		for (size_t i = 0; i < m_maxEntities; i++)
		{
			if (!isActive(i)) return i;
		}
		return -1;
	}

	Entity addEntity(const std::string& tag, const std::string& name);

	template <typename T>
	T& get(size_t entityId)
	{
		return std::get<std::vector<T>>(m_pool)[entityId];
	}

	template <typename T>
	bool has(size_t entityId)
	{
		return get<T>(entityId).exists;
	}

	template <typename T, typename... TArgs>
	T& add(size_t entityId, TArgs&&... mArgs)
	{
		auto& component = get<T>(entityId);
		component = T(std::forward<TArgs>(mArgs)...);
		component.exists = true;
		return component;
	}

	template <typename T>
	void remove(size_t entityId)
	{
		get<T>(entityId).exists = false;
	}

	std::string& tag(size_t entityId)
	{
		return m_tags[entityId];
	}
	std::string& name(size_t entityId)
	{
		return m_names[entityId];
	}
	bool isActive(size_t entityId)
	{
		return m_active[entityId];
	}
	void destroy(size_t entityId)
	{
		m_active[entityId] = false;
		m_numEntities--;
	}
};