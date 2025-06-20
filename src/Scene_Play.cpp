#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "ParticleSystem.hpp"
#include "Utils.hpp"
#include "PerlinNoise.hpp"

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
	registerKeyAction(sf::Keyboard::Scan::Left, "LEFT");
	registerKeyAction(sf::Keyboard::Scan::Right, "RIGHT");
	registerKeyAction(sf::Keyboard::Scan::Up, "UP");
	registerKeyAction(sf::Keyboard::Scan::Down, "DOWN");

	m_cameraView.setSize(sf::Vector2f(width(), height()));
	m_cameraView.zoom(1.0f);
	m_game->window().setView(m_cameraView);

	loadLevel(levelPath);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	m_entityManager = EntityManager();
	spawnPlayer();
	spawnTiles(filename);
	m_entityManager.update();
}

Entity Scene_Play::player()
{
	auto& player = m_entityManager.getEntities("player");
	assert(player.size() == 1);
	return player.front();
}

void Scene_Play::spawnPlayer()
{
	auto p = m_entityManager.addEntity("player", "playerCharacter");
	m_playerDied = false;
	
	auto& pAnimation = p.add<CAnimation>(m_game->assets().getAnimation("StormheadIdle"), true);

	Grid3D gridPos(0, 0, 0);
	auto& pTransform = p.add<CTransform>(Utils::gridToIsometric(gridPos, p));
	p.add<CGridPosition>(gridPos);
	p.add<CInput>();
}

void Scene_Play::spawnTiles(const std::string& filename)
{
	Noise2DArray whiteNoise = PerlinNoise::GenerateWhiteNoise(m_gridSize3D.x, m_gridSize3D.y);
	const static int octaveCount = 6;
	Noise2DArray perlinNoise = PerlinNoise::GeneratePerlinNoise(whiteNoise, octaveCount);

	auto& waterAni = m_game->assets().getAnimation("WaterTile");
	auto& groundAni = m_game->assets().getAnimation("GroundTile");
	auto& grassAni = m_game->assets().getAnimation("GrassTile");

	for (size_t i = 0; i < perlinNoise.size(); ++i)
	{
		for (size_t j = 0; j < perlinNoise[i].size(); ++j)
		{
			float noiseValue = perlinNoise[i][j];
			float height = std::round(noiseValue * m_gridSize3D.z);

			const static size_t waterLevel = 20;
			const static size_t grassLevel = 24;
			for (size_t k = height - 5; k <= height; k++)
			{
				if (k <= waterLevel)
				{
					spawnTile(i, j, k, waterAni);
				}
				else if (waterLevel < k && k < grassLevel)
				{
					spawnTile(i, j, k, groundAni);
				}
				else if (grassLevel <= k)
				{
					spawnTile(i, j, k, grassAni);
				}
			}
		}
	}
}

void Scene_Play::spawnTile(float gridX, float gridY, float gridZ, const Animation& animation)
{
	auto tile = m_entityManager.addEntity("tile", animation.m_name);
	tile.add<CAnimation>(animation, true);

	Grid3D gridPos(gridX, gridY, gridZ);
	tile.add<CTransform>(Utils::gridToIsometric(gridPos, tile));
	tile.add<CGridPosition>(gridPos);
	m_tileMap[gridPos] = tile;

	tile.add<CState>("unselected");
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
		sAnimation();
	}

	if (m_playerDied)
	{
		m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
	}
}

void Scene_Play::sMovement()
{
	static const float playerSpeed = 2.0f;

	auto& pInput = player().get<CInput>();
	auto& pTransform = player().get<CTransform>();

	pTransform.velocity = { 0.f, 0.f };

	if (pInput.left)  pTransform.velocity.x -= 1.f;
	if (pInput.right) pTransform.velocity.x += 1.f;
	if (pInput.up)    pTransform.velocity.y -= 1.f;
	if (pInput.down)  pTransform.velocity.y += 1.f;

	// Normalize if necessary
	if (pTransform.velocity.x != 0.f || pTransform.velocity.y != 0.f)
	{
		player().get<CState>().state = "running";
		pTransform.velocity = pTransform.velocity.normalize() * playerSpeed;
	}
	else
		player().get<CState>().state = "idle";


	for (Entity entity : m_entityManager.getEntities())
	{
		auto& eTransform = entity.get<CTransform>();

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
	auto& pTransform = player().get<CTransform>();
	auto visibleArea = Utils::visibleArea(m_cameraView);

	for (Entity tile : m_entityManager.getEntities("tile"))
	{
		if (player().id() == tile.id())
			continue;

		if (!Utils::isVisible(tile, visibleArea)) continue;

		auto overlap = Physics::GetOverlap(player(), tile);
		if (overlap.x > 0 && overlap.y > 0)
		{
			auto& tileTransform = tile.get<CTransform>();
			Vec2f prevOverlap = Physics::GetPreviousOverlap(player(), tile);
			if (prevOverlap.x > 0)
			{
				pTransform.velocity.y = 0;
				if (pTransform.prevPos.y < tileTransform.pos.y)
					pTransform.pos.y -= overlap.y;
				else
					pTransform.pos.y += overlap.y;
			}
			else if (prevOverlap.y > 0)
			{
				pTransform.velocity.x = 0;
				if (pTransform.prevPos.x < tileTransform.pos.x)
					pTransform.pos.x -= overlap.x;
				else
					pTransform.pos.x += overlap.x;
			}
		}
	}
}

void Scene_Play::sSelect()
{
	auto visibleArea = Utils::visibleArea(m_cameraView);

	for (Entity tile : m_entityManager.getEntities("tile"))
	{
		if (!Utils::isVisible(tile, visibleArea)) continue;

		auto& tileState = tile.get<CState>().state;
		bool insideTile = Utils::isInsideTopFace(m_mousePos, tile);
		if (insideTile && tileState != "selected")
			tileState = "selected";
		else if (!insideTile && tileState != "unselected")
			tileState = "unselected";
	}
}

void Scene_Play::sDoAction(const Action& action)
{
	auto& pInput = player().get<CInput>();
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
			sSelect();
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

void Scene_Play::sAnimation()
{
	auto visibleArea = Utils::visibleArea(m_cameraView);
	for (Entity tile : m_entityManager.getEntities("tile"))
	{
		if (!Utils::isVisible(tile, visibleArea)) continue;

		auto& transform = tile.get<CTransform>();
		auto& animation = tile.get<CAnimation>().animation;

		if (tile.get<CState>().state == "selected")
			animation.m_sprite.setPosition(transform.pos + Vec2f(0, -4.0f));
		else
			animation.m_sprite.setPosition(transform.pos);
		animation.update();
	}

	auto& transform = player().get<CTransform>();
	auto& animation = player().get<CAnimation>().animation;
	animation.m_sprite.setPosition(transform.pos);
}

void Scene_Play::sCamera()
{
	auto& pTransform = player().get<CTransform>();
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
	sf::Color clearColor = sf::Color(204, 226, 225);
	window.clear(clearColor);

	auto visibleArea = Utils::visibleArea(m_cameraView);
	for (Entity tile : m_entityManager.getEntities("tile"))
	{
		if (!Utils::isVisible(tile, visibleArea)) continue;
		if (Utils::isBehindAnotherTile(tile, m_tileMap)) continue;

		auto& animation = tile.get<CAnimation>().animation;
		window.draw(animation.m_sprite);
	}

	auto& animation = player().get<CAnimation>().animation;
	window.draw(animation.m_sprite);
}