#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class ParticleSystem
{
	struct Particle
	{
		sf::Vector2f velocity;
		int lifetime = 0;
	};
	
	std::vector<Particle> m_particles;
	sf::VertexArray m_vertices;
	sf::Vector2u m_windowSize;
	float m_size = 8;

	void resetParticle(size_t index, bool firstSpawn = false)
	{
		float mx = m_windowSize.x / 2;
		float my = m_windowSize.y / 2 + 32;

		m_vertices[6 * index + 0].position = sf::Vector2f(mx, my);
		m_vertices[6 * index + 1].position = sf::Vector2f(mx + m_size, my);
		m_vertices[6 * index + 2].position = sf::Vector2f(mx, my + m_size);
		m_vertices[6 * index + 3].position = sf::Vector2f(mx + m_size, my);
		m_vertices[6 * index + 4].position = sf::Vector2f(mx, my + m_size);
		m_vertices[6 * index + 5].position = sf::Vector2f(mx + m_size, my + m_size);

		sf::Color color(sf::Color::White);
		if (firstSpawn) color.a = 0;

		m_vertices[6 * index + 0].color = color;
		m_vertices[6 * index + 1].color = color;
		m_vertices[6 * index + 2].color = color;
		m_vertices[6 * index + 3].color = color;
		m_vertices[6 * index + 4].color = color;
		m_vertices[6 * index + 5].color = color;

		float rx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		float ry = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		m_particles[index].velocity = sf::Vector2f(rx, ry);

		m_particles[index].lifetime = 30 + rand() % 60;
	}

public:
	ParticleSystem() {}

	void resetParticles(size_t count = 16, float size = 4)
	{
		m_particles = std::vector<Particle>(count);
		m_vertices = sf::VertexArray(sf::PrimitiveType::Triangles, count * 3 * 2);
		m_size = size;

		for (size_t p = 0; p < m_particles.size(); p++)
		{
			resetParticle(p, true);
		}
	}

	void init(sf::Vector2u windowSize)
	{
		m_windowSize = windowSize;
		resetParticles();
	}

	void update()
	{
		for (size_t i = 0; i < m_particles.size(); i++)
		{
			if (m_particles[i].lifetime <= 0)
			{
				resetParticle(i);
			}
			m_particles[i].lifetime--;

			m_vertices[6 * i + 0].position += m_particles[i].velocity;
			m_vertices[6 * i + 1].position += m_particles[i].velocity;
			m_vertices[6 * i + 2].position += m_particles[i].velocity;
			m_vertices[6 * i + 3].position += m_particles[i].velocity;
			m_vertices[6 * i + 4].position += m_particles[i].velocity;
			m_vertices[6 * i + 5].position += m_particles[i].velocity;
		}
	}

	void draw(sf::RenderWindow& window) const
	{
		window.draw(m_vertices);
	}
};