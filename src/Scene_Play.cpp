#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "ParticleSystem.hpp"
#include "Utils.hpp"
#include "Entity.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <math.h>
#include <algorithm>

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerMouseAction(sf::Mouse::Button::Left, "LEFT_CLICK");
	registerMouseAction(sf::Mouse::Button::Right, "RIGHT_CLICK");

	registerKeyAction(sf::Keyboard::Scan::Escape, "ESCAPE");

	registerKeyAction(sf::Keyboard::Scan::A, "LEFT");
	registerKeyAction(sf::Keyboard::Scan::D, "RIGHT");
	registerKeyAction(sf::Keyboard::Scan::W, "UP");
	registerKeyAction(sf::Keyboard::Scan::S, "DOWN");

	m_cameraView.setSize(sf::Vector2f(width(), height()));
	m_cameraView.zoom(1.0f);
	m_game->window().setView(m_cameraView);

	loadLevel(levelPath);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	const static size_t MAX_ENTITIES = 1000;

	m_entityManager = EntityManager();
	m_memoryPool = MemoryPool(MAX_ENTITIES);
	spawnPlayer();
	spawnLevelEntities();
	m_entityManager.update(m_memoryPool);
}

Entity Scene_Play::player()
{
	auto& player = m_entityManager.getEntities("player");
	assert(player.size() == 1);
	return player.front();
}

void Scene_Play::spawnPlayer()
{
	auto p = m_entityManager.addEntity(m_memoryPool, "player", "PlayerCharacter");
	m_playerDied = false;
	
	auto& pAnimation = p.add<CAnimation>(m_memoryPool, m_game->assets().getAnimation("StormheadIdle"), true);
	p.add<CTransform>(m_memoryPool, Vec2f(0, 0));
	p.add<CInput>(m_memoryPool);
}

void Scene_Play::spawnLevelEntities()
{
	auto enemy = m_entityManager.addEntity(m_memoryPool, "enemy", "enemyCharacter");
	enemy.add<CAnimation>(m_memoryPool, m_game->assets().getAnimation("StormheadIdle"), true);
	enemy.add<CTransform>(m_memoryPool, Vec2f(200, 200));
}

void Scene_Play::update()
{
	if (!m_paused)
	{
		m_entityManager.update(m_memoryPool);
		sMovement();
		sCollision();
		sCamera();
		sAnimation();
	}

	if (m_playerDied)
	{
		m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
	}
}

void Scene_Play::sMovement()
{
	static const float moveStep = 5.0f;

	auto p = player();
	auto& input = p.get<CInput>(m_memoryPool);
	auto& transform = p.get<CTransform>(m_memoryPool);

	transform.velocity = { 0, 0 };

	if (input.right) { transform.velocity.x += moveStep; } // SW
	if (input.left) { transform.velocity.x -= moveStep; } // NE
	if (input.down) { transform.velocity.y += moveStep; } // UP
	if (input.up) { transform.velocity.y -= moveStep; } // DOWN

	if (!(transform.velocity == Vec2f(0, 0)))
	{
		transform.prevPos = transform.pos;
		transform.pos += transform.velocity;
	}
}

void Scene_Play::sCollision()
{
	
}

void Scene_Play::sAnimation()
{
	for (Entity entity : m_entityManager.getEntities())
	{
		auto& transform = entity.get<CTransform>(m_memoryPool);
		auto& animation = entity.get<CAnimation>(m_memoryPool).animation;
		animation.m_sprite.setPosition(transform.pos);
		animation.update();
	}
}

void Scene_Play::sCamera()
{
	auto& pTransform = player().get<CTransform>(m_memoryPool);
	m_cameraView.setCenter(pTransform.pos);
	m_game->window().setView(m_cameraView);
}

void Scene_Play::sRender()
{
	auto& window = m_game->window();
	sf::Color clearColor = sf::Color(204, 226, 225);
	window.clear(clearColor);

	for (Entity entity : m_entityManager.getEntities())
	{
		auto& animation = entity.get<CAnimation>(m_memoryPool).animation;
		window.draw(animation.m_sprite);
	}
}

void Scene_Play::onEnd()
{
	m_game->quit();
}

void Scene_Play::onExitScene()
{

}

void Scene_Play::onEnterScene()
{
	auto& window = m_game->window();
	window.setView(m_cameraView);
}

void Scene_Play::sDoAction(const Action& action)
{
	auto& pInput = player().get<CInput>(m_memoryPool);
	if (action.m_type == "START")
	{
		if (action.m_name == "LEFT")
			pInput.left = true;
		else if (action.m_name == "RIGHT")
			pInput.right = true;
		else if (action.m_name == "UP")
			pInput.up = true;
		else if (action.m_name == "DOWN")
			pInput.down = true;
		else if (action.m_name == "ESCAPE")
		{
			m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
		}
		else if (action.m_name == "LEFT_CLICK")
		{
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
		else if (action.m_name == "RIGHT_CLICK")
		{
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
		else if (action.m_name == "MOUSE_MOVE")
		{
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
		else if (action.m_name == "MOUSE_SCROLL")
		{
			float zoomFactor = action.m_mouseScrollDelta > 0 ? 0.9f : 1.1f;
			m_cameraView.zoom(zoomFactor);
			m_game->window().setView(m_cameraView);
		}
	}
	else if (action.m_type == "END")
	{
		if (action.m_name == "LEFT")
			pInput.left = false;
		else if (action.m_name == "RIGHT")
			pInput.right = false;
		if (action.m_name == "UP")
			pInput.up = false;
		else if (action.m_name == "DOWN")
			pInput.down = false;
	}
}