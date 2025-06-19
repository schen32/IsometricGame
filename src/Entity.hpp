#pragma once

#include "Components.hpp"
#include <string>
#include <tuple>

class EntityManager;

using ComponentTuple = std::tuple<
	CTransform,
	CGridPosition,
	CInput,
	CBoundingBox,
	CAnimation,
	CState,
	CHealth,
	CDamage
>;

class Entity
{
	friend class EntityManager;

	ComponentTuple m_components;
	bool m_active = true;
	size_t m_id = 0;

	Entity(const std::string& tag, const std::string& name, const size_t& id)
		: m_tag(tag), m_name(name), m_id(id) {}

public:
	std::string m_tag = "default";
	std::string m_name = "name";

	bool isActive() const
	{
		return m_active;
	}

	void destroy()
	{
		m_active = false;
	}

	size_t id() const
	{
		return m_id;
	}

	const std::string& tag() const
	{
		return m_tag;
	}

	const std::string& name() const
	{
		return m_name;
	}

	template <typename T>
	bool has() const
	{
		return get<T>().exists;
	}

	template <typename T, typename... TArgs>
	T& add(TArgs&&... mArgs)
	{
		auto& component = get<T>();
		component = T(std::forward<TArgs>(mArgs)...);
		component.exists = true;
		return component;
	}

	template <typename T>
	T& get()
	{
		return std::get<T>(m_components);
	}

	template <typename T>
	const T& get() const
	{
		return std::get<T>(m_components);
	}

	template <typename T>
	void remove()
	{
		get<T>() = T();
	}
};