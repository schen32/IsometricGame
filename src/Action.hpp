#pragma once

#include <sstream>
#include <string>
#include "Vec2.hpp"

class Action
{
public:
	std::string m_name = "NONE";
	std::string m_type = "NONE";
	Vec2f m_mousePos = { 0, 0 };
	float m_mouseScrollDelta = 0.0f;

	Action() = default;
	Action(const std::string& name, const std::string& type)
		: m_name(name), m_type(type) { }
	Action(const std::string& name, const std::string& type, const Vec2f& mousePos)
		: m_name(name), m_type(type), m_mousePos(mousePos) {}
	Action(const std::string& name, const std::string& type, float msd)
		: m_name(name), m_type(type), m_mouseScrollDelta(msd) {
	}
};