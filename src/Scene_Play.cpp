#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "ParticleSystem.hpp"
#include "Utils.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <math.h>

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
	registerKeyAction(sf::Keyboard::Scan::Left, "LEFT");
	registerKeyAction(sf::Keyboard::Scan::Right, "RIGHT");
	registerKeyAction(sf::Keyboard::Scan::Up, "UP");
	registerKeyAction(sf::Keyboard::Scan::Down, "DOWN");

	m_cameraView.setSize(sf::Vector2f(width(), height()));
	m_cameraView.zoom(0.5f);
	m_game->window().setView(m_cameraView);

	loadLevel(levelPath);
}

Vec2f Scene_Play::gridToIsometric(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	auto& eAnimation = entity->get<CAnimation>();
	Vec2f eSize = eAnimation.animation.m_size;

	Vec2f i = Vec2f(eSize.x / 2, 0.5f * eSize.y / 2);
	Vec2f j = Vec2f(-eSize.x / 2, 0.5f * eSize.y / 2);
	
	return (i * gridX + j * gridY);
}

Vec2f Scene_Play::isometricToGrid(float isoX, float isoY)
{
	Vec2f eSize = m_gridCellSize;

	Vec2f i = Vec2f(eSize.x / 2, 0.5f * eSize.y / 2);
	Vec2f j = Vec2f(-eSize.x / 2, 0.5f * eSize.y / 2);

	// Matrix elements
	float a = i.x, b = j.x;
	float c = i.y, d = j.y;

	float det = a * d - b * c;
	if (det == 0.0f) {
		std::cerr << "Invalid basis vectors: determinant is zero!\n";
		return Vec2f(0.f, 0.f);
	}

	float invDet = 1.0f / det;

	// Apply inverse matrix
	float gridX = invDet * (d * isoX - b * isoY);
	float gridY = invDet * (-c * isoX + a * isoY);

	return Vec2f(gridX, gridY);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	m_entityManager = EntityManager();
	spawnTiles(filename);
	spawnPlayer();
	m_entityManager.update();
}

std::shared_ptr<Entity> Scene_Play::player()
{
	auto& player = m_entityManager.getEntities("player");
	assert(player.size() == 1);
	return player.front();
}

void Scene_Play::spawnPlayer()
{
	auto p = m_entityManager.addEntity("player", "playerCharacter");
	m_playerDied = false;
	
	auto& pAnimation = p->add<CAnimation>(m_game->assets().getAnimation("StormheadIdle"), true);
	auto& pTransform = p->add<CTransform>();
	p->add<CInput>();
}

void Scene_Play::spawnTiles(const std::string& filename)
{
	/*auto file = std::ifstream(filename);
	while (file.good())
	{
		std::string type;
		file >> type;
		if (type == "Tile")
		{
			std::string aniName;
			float gridX, gridY;
			file >> aniName >> gridX >> gridY;

			spawnTile(gridX, gridY, aniName);
		}
	}
	file.close();*/

	for (int i = -m_gridSize.x / 2; i < m_gridSize.x / 2; i++)
	{
		for (int j = -m_gridSize.y / 2; j < m_gridSize.y / 2; j++)
		{
			spawnTile(i, j, "SandTile");
		}
	}
}

void Scene_Play::spawnTile(float gridX, float gridY, const std::string& aniName)
{
	auto tile = m_entityManager.addEntity("tile", aniName);
	tile->add<CAnimation>(m_game->assets().getAnimation(aniName), true);
	tile->add<CTransform>(gridToIsometric(gridX, gridY, tile));
	tile->add<CGridPosition>(gridX, gridY);
	tile->add<CState>("unselected");
}

void Scene_Play::update()
{
	if (!m_paused)
	{
		m_entityManager.update();
		sAI();
		sMovement();
		sCollision();
		sCamera();
		sSelect();
		sAnimation();
	}

	if (m_playerDied)
	{
		m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
	}
}

void Scene_Play::sSelect()
{
	Vec2f selectedGridCell = isometricToGrid(m_mousePos.x, m_mousePos.y);
	for (auto& tile : m_entityManager.getEntities("tile"))
	{
		auto& gridPos = tile->get<CGridPosition>();
		auto& tileState = tile->get<CState>().state;
		bool selected = static_cast<int>(selectedGridCell.x) == gridPos.x &&
			static_cast<int>(selectedGridCell.y) == gridPos.y;

		if (selected && tileState != "selected")
			tileState = "selected";
		else if (!selected && tileState != "unselected")
			tileState = "unselected";
	}
}

void Scene_Play::sMovement()
{
	auto& pInput = player()->get<CInput>();
	auto& pTransform = player()->get<CTransform>();

	pTransform.velocity = { 0.f, 0.f };

	if (pInput.left)  pTransform.velocity.x -= 1.f;
	if (pInput.right) pTransform.velocity.x += 1.f;
	if (pInput.up)    pTransform.velocity.y -= 1.f;
	if (pInput.down)  pTransform.velocity.y += 1.f;

	// Normalize if necessary
	if (pTransform.velocity.x != 0.f || pTransform.velocity.y != 0.f)
	{
		player()->get<CState>().state = "running";
		pTransform.velocity = pTransform.velocity.normalize() * 1.f;
	}
	else
		player()->get<CState>().state = "idle";


	for (auto& entity : m_entityManager.getEntities())
	{
		auto& eTransform = entity->get<CTransform>();

		eTransform.prevPos = eTransform.pos;
		eTransform.pos += eTransform.velocity;
	}
}

void Scene_Play::sAI()
{

}

void Scene_Play::sStatus()
{

}

void Scene_Play::sCollision()
{
	for (auto& e1 : m_entityManager.getEntities())
	{
		for (auto& e2 : m_entityManager.getEntities())
		{
			if (e1->id() == e2->id())
				continue;

			auto overlap = Physics::GetOverlap(e1, e2);
			if (overlap.x > 0 && overlap.y > 0)
			{
				auto& e1Transform = e1->get<CTransform>();
				auto& e2Transform = e2->get<CTransform>();
				Vec2f prevOverlap = Physics::GetPreviousOverlap(e1, e2);
				if (prevOverlap.x > 0)
				{
					e1Transform.velocity.y = 0;
					if (e1Transform.prevPos.y < e2Transform.pos.y)
						e1Transform.pos.y -= overlap.y;
					else
						e1Transform.pos.y += overlap.y;
				}
				else if (prevOverlap.y > 0)
				{
					e1Transform.velocity.x = 0;
					if (e1Transform.prevPos.x < e2Transform.pos.x)
						e1Transform.pos.x -= overlap.x;
					else
						e1Transform.pos.x += overlap.x;
				}
			}
		}
	}
}

void Scene_Play::sDoAction(const Action& action)
{
	auto& pInput = player()->get<CInput>();
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
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
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

void Scene_Play::sAnimation()
{
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CAnimation>())
			continue;

		auto& eAnimation = entity->get<CAnimation>();
		eAnimation.animation.update();

		if (!eAnimation.repeat && eAnimation.animation.hasEnded())
		{
			entity->destroy();
			continue;
		}
	}
}

void Scene_Play::sCamera()
{
	auto& pTransform = player()->get<CTransform>();
	m_cameraView.setCenter(pTransform.pos);
	m_game->window().setView(m_cameraView);
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

void Scene_Play::sGui()
{

}

void Scene_Play::sRender()
{
	auto& window = m_game->window();
	window.clear(sf::Color(204, 226, 225));

	for (auto& tile : m_entityManager.getEntities("tile"))
	{
		auto& transform = tile->get<CTransform>();
		auto& animation = tile->get<CAnimation>().animation;

		if (tile->get<CState>().state == "selected")
		{
			animation.m_sprite.setPosition(transform.pos + Vec2f(0, -4.0f));
		}
		else
		{
			animation.m_sprite.setPosition(transform.pos);
		}
		window.draw(animation.m_sprite);
	}

	auto& transform = player()->get<CTransform>();
	auto& animation = player()->get<CAnimation>().animation;

	animation.m_sprite.setPosition(transform.pos);
	window.draw(animation.m_sprite);
}