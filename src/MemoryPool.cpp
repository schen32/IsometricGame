#include "MemoryPool.hpp"
#include "Entity.hpp"

size_t MemoryPool::getNextEntityIndex()
{
	if (m_freeIndices.empty())
	{
		throw std::runtime_error("No more free entity indices!");
	}
	size_t index = m_freeIndices.back();
	m_freeIndices.pop_back();
	return index;
}

Entity MemoryPool::addEntity(const std::string& tag, const std::string& name)
{
	// this doesn't work
	//size_t index = getNextEntityIndex();

	size_t index = m_freeIndices.back();
	m_freeIndices.pop_back();

	std::apply([&](auto&... componentVecs) {
		(..., (componentVecs[index].exists = false));
		}, m_pool);

	m_tags[index] = tag;
	m_names[index] = name;
	m_active[index] = true;
	m_numEntities++;
	return Entity(index);
}