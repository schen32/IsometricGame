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

	bool isActive()
	{
		return MemoryPool::Instance().isActive(m_id);
	}

	void destroy()
	{
		MemoryPool::Instance().destroy(m_id);
	}

	size_t id()
	{
		return m_id;
	}

	std::string& tag()
	{
		return MemoryPool::Instance().tag(m_id);
	}

	std::string& name()
	{
		return MemoryPool::Instance().name(m_id);
	}

	template <typename T>
	bool has()
	{
		return MemoryPool::Instance().has<T>(m_id);
	}

	template <typename T, typename... TArgs>
	T& add(TArgs&&... mArgs)
	{
		return MemoryPool::Instance().add<T>(m_id, std::forward<TArgs>(mArgs)...);
	}

	template <typename T>
	T& get()
	{
		return MemoryPool::Instance().get<T>(m_id);
	}

	template <typename T>
	void remove()
	{
		MemoryPool::Instance().remove<T>(m_id);
	}
};