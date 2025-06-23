#pragma once

#include <tuple>
#include <vector>
#include "Components.hpp"

using ComponentVectorTuple = std::tuple<
	std::vector<std::optional<CTransform>>,
	std::vector<std::optional<CGridPosition>>,
	std::vector<std::optional<CChunkTiles>>,
	std::vector<std::optional<CTileRenderInfo>>,
	std::vector<std::optional<CVertexArray>>,
	std::vector<std::optional<CInput>>,
	std::vector<std::optional<CBoundingBox>>,
	std::vector<std::optional<CAnimation>>,
	std::vector<std::optional<CState>>,
	std::vector<std::optional<CHealth>>,
	std::vector<std::optional<CDamage>>
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
	std::vector<size_t>			m_freeIndices;
	bool						m_entityDestroyed = false;

	MemoryPool() = default;
	MemoryPool(size_t maxEntities) : m_numEntities(0), m_maxEntities(maxEntities)
	{
		std::apply([&](auto&... componentVecs) {
			(..., componentVecs.resize(maxEntities));
			}, m_pool);

		m_tags.resize(maxEntities);
		m_names.resize(maxEntities);
		m_active.resize(maxEntities);
		m_freeIndices.reserve(maxEntities);
		for (int i = 0; i < m_maxEntities; i++)
		{
			m_freeIndices.emplace_back(i);
		}
	}

	size_t getNextEntityIndex();
	Entity addEntity(const std::string& tag, const std::string& name);

	template <typename T>
	T& get(size_t entityId)
	{
		auto& opt = std::get<std::vector<std::optional<T>>>(m_pool)[entityId];
		if (!opt.has_value())
			throw std::runtime_error("Component not found for entity " + std::to_string(entityId));
		return *opt;
	}

	template <typename T>
	bool has(size_t entityId)
	{
		auto& opt = std::get<std::vector<std::optional<T>>>(m_pool)[entityId];
		return opt.has_value();
	}

	template <typename T, typename... TArgs>
	T& add(size_t entityId, TArgs&&... mArgs)
	{
		auto& opt = std::get<std::vector<std::optional<T>>>(m_pool)[entityId];
		opt.emplace(std::forward<TArgs>(mArgs)...);
		return *opt;
	}

	template <typename T>
	void remove(size_t entityId)
	{
		auto& opt = std::get<std::vector<std::optional<T>>>(m_pool)[entityId];
		opt.reset();
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
		m_freeIndices.emplace_back(entityId);
		m_numEntities--;
		m_entityDestroyed = true;
	}
};