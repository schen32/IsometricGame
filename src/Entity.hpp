#pragma once

#include "Components.hpp"
#include "MemoryPool.hpp"
#include <string>

class Entity
{
public:
	size_t m_id = 0;

	Entity() = default;
	Entity(size_t id) : m_id(id) {}

	bool isActive(MemoryPool& pool)
	{
		return pool.isActive(m_id);
	}

	void destroy(MemoryPool& pool)
	{
		pool.destroy(m_id);
	}

	size_t id()
	{
		return m_id;
	}

	std::string& tag(MemoryPool& pool)
	{
		return pool.tag(m_id);
	}

	std::string& name(MemoryPool& pool)
	{
		return pool.name(m_id);
	}

	template <typename T>
	bool has(MemoryPool& pool)
	{
		return pool.has<T>(m_id);
	}

	template <typename T, typename... TArgs>
	T& add(MemoryPool& pool, TArgs&&... mArgs)
	{
		return pool.add<T>(m_id, std::forward<TArgs>(mArgs)...);
	}

	template <typename T>
	T& get(MemoryPool& pool)
	{
		return pool.get<T>(m_id);
	}

	template <typename T>
	void remove(MemoryPool& pool)
	{
		pool.remove<T>(m_id);
	}
};