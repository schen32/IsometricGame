#include "MemoryPool.hpp"
#include "Entity.hpp"

Entity MemoryPool::addEntity(const std::string& tag, const std::string& name)
{
	size_t index = getNextEntityIndex();

	std::apply([&](auto&... componentVecs) {
		(..., (componentVecs[index].exists = false));
		}, m_pool);

	m_tags[index] = tag;
	m_names[index] = name;
	m_active[index] = true;
	m_numEntities++;
	return Entity(index);
}