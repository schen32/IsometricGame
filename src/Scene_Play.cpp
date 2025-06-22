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

	registerKeyAction(sf::Keyboard::Scan::Space, "UP");
	registerKeyAction(sf::Keyboard::Scan::LShift, "DOWN");

	registerKeyAction(sf::Keyboard::Scan::A, "LEFT");
	registerKeyAction(sf::Keyboard::Scan::D, "RIGHT");
	registerKeyAction(sf::Keyboard::Scan::W, "FORWARD");
	registerKeyAction(sf::Keyboard::Scan::S, "BACKWARD");

	m_cameraView.setSize(sf::Vector2f(width(), height()));
	m_cameraView.zoom(1.0f);
	m_game->window().setView(m_cameraView);

	loadLevel(levelPath);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	const static size_t MAX_CHUNKS = m_numChunks3D.volume() * m_chunkSize3D.volume() * 2;

	m_entityManager = EntityManager();
	m_memoryPool = MemoryPool(MAX_CHUNKS);
	spawnPlayer();
	// spawnTiles();
	spawnChunks();
	m_entityManager.update(m_memoryPool);
	buildVertexArraysForChunks();
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

	Grid3D gridPos(0, 0, 0);
	p.add<CTransform>(m_memoryPool, Utils::gridToIsometric(gridPos, m_gridCellSize));
	p.add<CGridPosition>(m_memoryPool, gridPos);
	p.add<CInput>(m_memoryPool);
}

void Scene_Play::buildVertexArraysForChunks()
{
	for (Entity chunk : m_entityManager.getEntities("chunk"))
	{
		auto& chunkTiles = chunk.get<CChunkTiles>(m_memoryPool);
		auto chunkVertexArray = buildVertexArrayForChunk(chunkTiles, m_game->assets().getTexture("TexTiles"));
		chunk.add<CVertexArray>(m_memoryPool, chunkVertexArray);
	}
}

void Scene_Play::spawnChunks()
{
	for (size_t i = 0; i < m_numChunks3D.x; i++)
	{
		for (size_t j = 0; j < m_numChunks3D.y; j++)
		{
			for (size_t k = 0; k < m_numChunks3D.z; k++)
			{
				spawnChunk(i, j, k);
			}
		}
	}
}

void Scene_Play::spawnChunk(float chunkX, float chunkY, float chunkZ)
{
	auto chunk = m_entityManager.addEntity(m_memoryPool, "chunk", "TileChunk");

	Grid3D gridPos(chunkX * m_chunkSize3D.x + m_chunkSize3D.x / 2,
				   chunkY * m_chunkSize3D.y + m_chunkSize3D.y / 2,
		           chunkZ * m_chunkSize3D.z + m_chunkSize3D.z / 2);

	chunk.add<CTransform>(m_memoryPool, Utils::gridToIsometric(gridPos, m_gridCellSize));
	auto& chunkPos = chunk.add<CGridPosition>(m_memoryPool, gridPos);
	auto& chunkTiles = chunk.add<CChunkTiles>(m_memoryPool);

	spawnTilesFromChunk(chunkPos, chunkTiles);
}

void Scene_Play::spawnTilesFromChunk(const CGridPosition& chunkPos, CChunkTiles& chunkTiles)
{
	chunkTiles.tiles.reserve(m_chunkSize3D.volume());

	Grid3D cPos = chunkPos.pos;
	Grid3D halfChunkSize = m_chunkSize3D / 2;
	for (size_t i = cPos.x - halfChunkSize.x; i < cPos.x + halfChunkSize.x; i++)
	{
		for (size_t j = cPos.y - halfChunkSize.y; j < cPos.y + halfChunkSize.y; j++)
		{
			for (size_t k = cPos.z - halfChunkSize.z; k < cPos.z + halfChunkSize.z; k++)
			{
				chunkTiles.tiles.emplace_back(spawnTile(i, j, k));
			}
		}
	}
}

void Scene_Play::spawnTiles()
{
	Noise2DArray whiteNoise = PerlinNoise::GenerateWhiteNoise(m_gridSize3D.x, m_gridSize3D.y);
	const static int octaveCount = 6;
	Noise2DArray perlinNoise = PerlinNoise::GeneratePerlinNoise(whiteNoise, octaveCount);

	for (size_t i = 0; i < perlinNoise.size(); ++i)
	{
		for (size_t j = 0; j < perlinNoise[i].size(); ++j)
		{
			float noiseValue = perlinNoise[i][j];
			float height = std::round(noiseValue * m_gridSize3D.z);
			for (size_t k = height - 3; k <= height; k++)
			{
				spawnTile(i, j, k);
			}
		}
	}
}

Entity Scene_Play::spawnTile(float gridX, float gridY, float gridZ)
{
	const static size_t waterLevel = 20;
	const static size_t grassLevel = 24;

	sf::Vector2i tileTexPos = (gridZ <= 20) ? sf::Vector2i(0, 10) :
							  (gridZ <= 24) ? sf::Vector2i(0, 0) :
											  sf::Vector2i(0, 2);
	tileTexPos.x *= m_gridCellSize.x;
	tileTexPos.y *= m_gridCellSize.y;

	auto tile = m_entityManager.addEntity(m_memoryPool, "tile", "Tile");
	Grid3D gridPos(gridX, gridY, gridZ);
	
	tile.add<CTileRenderInfo>(m_memoryPool, Utils::gridToIsometric(gridPos, m_gridCellSize),
		sf::IntRect(tileTexPos, sf::Vector2i(m_gridCellSize)));
	tile.add<CGridPosition>(m_memoryPool, gridPos);

	m_tileMap.insert({ gridPos, tile });
	return tile;
}

void Scene_Play::update()
{
	if (!m_paused)
	{
		m_entityManager.update(m_memoryPool);
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
	static const float moveStep = 0.5f;

	auto p = player();
	auto& input = p.get<CInput>(m_memoryPool);
	auto& transform = p.get<CTransform>(m_memoryPool);
	auto& grid = p.get<CGridPosition>(m_memoryPool);

	Grid3D delta = { 0, 0, 0 };

	if (input.forward) { delta.x -= moveStep; delta.y -= moveStep; } // NW
	if (input.backward) { delta.x += moveStep; delta.y += moveStep; } // SE
	if (input.left) { delta.x -= moveStep; delta.y += moveStep; } // SW
	if (input.right) { delta.x += moveStep; delta.y -= moveStep; } // NE
	if (input.up) { delta.z += moveStep; } // UP
	if (input.down) { delta.z -= moveStep; } // DOWN

	if (!(delta == Grid3D{ 0, 0, 0 })) {
		grid.pos += delta;

		transform.prevPos = transform.pos;
		transform.pos = Utils::gridToIsometric(grid.pos, m_gridCellSize);
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
	
}

void Scene_Play::sSelect()
{
	
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
		if (action.m_name == "FORWARD")
			pInput.forward = true;
		else if (action.m_name == "BACKWARD")
			pInput.backward = true;
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
		if (action.m_name == "FORWARD")
			pInput.forward = false;
		else if (action.m_name == "BACKWARD")
			pInput.backward = false;
	}
}

void Scene_Play::sAnimation()
{
	auto& transform = player().get<CTransform>(m_memoryPool);
	auto& animation = player().get<CAnimation>(m_memoryPool).animation;
	animation.m_sprite.setPosition(transform.pos);
}

void Scene_Play::sCamera()
{
	auto& pTransform = player().get<CTransform>(m_memoryPool);
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
	for (Entity chunk : m_entityManager.getEntities("chunk"))
	{
		auto& cTrans = chunk.get<CTransform>(m_memoryPool);
		if (!Utils::isVisible(cTrans, visibleArea)) continue;

		auto& chunkVertexArray = chunk.get<CVertexArray>(m_memoryPool).va;
		window.draw(chunkVertexArray, &m_game->assets().getTexture("TexTiles"));
	}

	auto& animation = player().get<CAnimation>(m_memoryPool).animation;
	window.draw(animation.m_sprite);

	window.setView(window.getDefaultView());

	auto& pGridPos = player().get<CGridPosition>(m_memoryPool).pos;
	sf::Text pGridPosText(m_game->assets().getFont("FutureMillennium"), pGridPos.toString());
	window.draw(pGridPosText);

	window.setView(m_cameraView);
}

sf::VertexArray Scene_Play::buildVertexArrayForChunk(CChunkTiles& tileChunk, const sf::Texture& tileset)
{
	auto& tiles = tileChunk.tiles;
	sf::VertexArray va(sf::PrimitiveType::Triangles); // No pre-sizing for dynamic appending

	for (size_t i = 0; i < tiles.size(); ++i)
	{
		Entity& tile = tiles[i];
		auto& tileGridPos = tile.get<CGridPosition>(m_memoryPool).pos;

		// Optionally skip tiles that are fully enclosed
		if (m_tileMap.find(tileGridPos + Grid3D(1, 1, 1)) != m_tileMap.end())
			continue;

		auto& tileInfo = tile.get<CTileRenderInfo>(m_memoryPool);
		const sf::Vector2f& pos = tileInfo.position;
		const sf::Vector2f origin = m_gridCellSize / 2.f;
		const sf::IntRect& texRect = tileInfo.textureRect;

		// Vertex positions in world space
		sf::Vector2f topLeft = pos - origin;
		sf::Vector2f topRight = { topLeft.x + texRect.size.x, topLeft.y };
		sf::Vector2f bottomRight = { topLeft.x + texRect.size.x, topLeft.y + texRect.size.y };
		sf::Vector2f bottomLeft = { topLeft.x, topLeft.y + texRect.size.y };

		// Texture coordinates
		sf::Vector2f texTopLeft(texRect.position.x, texRect.position.y);
		sf::Vector2f texTopRight(texRect.position.x + texRect.size.x, texRect.position.y);
		sf::Vector2f texBottomRight(texRect.position.x + texRect.size.x, texRect.position.y + texRect.size.y);
		sf::Vector2f texBottomLeft(texRect.position.x, texRect.position.y + texRect.size.y);

		// First triangle
		va.append(sf::Vertex({ topLeft, sf::Color::White, texTopLeft }));
		va.append(sf::Vertex({ topRight, sf::Color::White, texTopRight }));
		va.append(sf::Vertex({ bottomRight, sf::Color::White, texBottomRight }));

		// Second triangle
		va.append(sf::Vertex({ bottomRight, sf::Color::White, texBottomRight }));
		va.append(sf::Vertex({ bottomLeft, sf::Color::White, texBottomLeft }));
		va.append(sf::Vertex({ topLeft, sf::Color::White, texTopLeft }));
	}

	return va;
}
