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
	generateHeightMap();
}

void Scene_Play::loadLevel(const std::string& filename)
{
	const static size_t MAX_CHUNKS = m_numChunks3D.volume() * m_chunkSize3D.volume() * m_loadRadius;

	m_entityManager = EntityManager();
	m_memoryPool = MemoryPool(MAX_CHUNKS);
	spawnPlayer();
	m_entityManager.update(m_memoryPool);
}

void Scene_Play::generateHeightMap()
{
	int w = m_gridSize3D.x, h = m_gridSize3D.y, maxZ = int(m_gridSize3D.z);

	// flat storage
	m_heightMap.clear();
	m_heightMap.reserve(w * h);

	Noise2DArray whiteNoise = PerlinNoise::GenerateWhiteNoise(w, h);
	Noise2DArray perlinNoise = PerlinNoise::GeneratePerlinNoise(whiteNoise, 6);

	for (int j = 0; j < h; ++j)            // note: row (y) outer for cache
	{
		auto& row = perlinNoise[j];
		for (int i = 0; i < w; ++i)        // column (x) inner
		{
			float v = row[i] * maxZ;
			m_heightMap.emplace_back(int(v + 0.5f));  // round
		}
	}
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

	Grid3D gridPos(0, 0, m_chunkSize3D.z);
	p.add<CTransform>(m_memoryPool, Utils::gridToIsometric(gridPos, m_gridCellSize));
	p.add<CGridPosition>(m_memoryPool, gridPos);
	p.add<CInput>(m_memoryPool);
}

void Scene_Play::buildVertexArraysForChunks()
{
	if (!m_chunkChanged) return;
	m_chunkChanged = false;

	for (Entity chunk : m_entityManager.getEntities("chunk"))
	{
		auto& chunkTiles = chunk.get<CChunkTiles>(m_memoryPool);
		if (!chunkTiles.changed) continue;

		auto chunkVertexArray = buildVertexArrayForChunk(chunkTiles, m_game->assets().getTexture("TexTiles"));
		chunk.add<CVertexArray>(m_memoryPool, chunkVertexArray);
	}
}

void Scene_Play::spawnChunks()
{
	auto playerChunkPos = Utils::gridToChunkPos(player().get<CGridPosition>(m_memoryPool), m_chunkSize3D);
	for (int dx = -m_loadRadius; dx <= m_loadRadius; ++dx)
	{
		for (int dy = -m_loadRadius; dy <= m_loadRadius; ++dy)
		{
			for (int dz = -m_loadRadius; dz <= m_loadRadius; ++dz)
			{
				Grid3D chunkPos = playerChunkPos + Grid3D(dx, dy, dz);
				if (m_chunkMap.find(chunkPos) != m_chunkMap.end()) continue;

				Entity chunk = spawnChunk(chunkPos);
				m_chunkMap.insert({ chunkPos, chunk });
				m_chunkChanged = true;
			}
		}
	}
}

void Scene_Play::despawnChunks()
{
	auto playerChunkPos = Utils::gridToChunkPos(player().get<CGridPosition>(m_memoryPool), m_chunkSize3D);
	for (Entity chunk : m_entityManager.getEntities("chunk"))
	{
		auto chunkPos = Utils::gridToChunkPos(chunk.get<CGridPosition>(m_memoryPool), m_chunkSize3D);
		int dx = std::abs(chunkPos.x - playerChunkPos.x);
		int dy = std::abs(chunkPos.y - playerChunkPos.y);
		int dz = std::abs(chunkPos.z - playerChunkPos.z);

		if (!(dx > m_loadRadius || dy > m_loadRadius || dz > m_loadRadius)) continue;

		auto& tileChunk = chunk.get<CChunkTiles>(m_memoryPool);
		for (Entity tile : tileChunk.tiles)
		{
			auto& tilePos = tile.get<CGridPosition>(m_memoryPool);
			tile.destroy(m_memoryPool);
			m_tileMap.erase(tilePos.pos);
		}
		chunk.destroy(m_memoryPool);
		m_chunkMap.erase(chunkPos);
	}
}

Entity Scene_Play::spawnChunk(const Grid3D& chunkPos)
{
	auto chunk = m_entityManager.addEntity(m_memoryPool, "chunk", "TileChunk");

	Grid3D gridPos(chunkPos.x * m_chunkSize3D.x,
				   chunkPos.y * m_chunkSize3D.y,
				   chunkPos.z * m_chunkSize3D.z);

	chunk.add<CTransform>(m_memoryPool, Utils::gridToIsometric(gridPos, m_gridCellSize));
	auto& chunkGridPos = chunk.add<CGridPosition>(m_memoryPool, gridPos);
	auto& chunkTiles = chunk.add<CChunkTiles>(m_memoryPool);

	spawnTilesFromChunk(chunkGridPos, chunkTiles);
	chunkTiles.changed = true;
	return chunk;
}

void Scene_Play::spawnTilesFromChunk(const CGridPosition& chunkGridPos, CChunkTiles& chunkTiles)
{
	int w = m_gridSize3D.x, h = m_gridSize3D.y;
	Grid3D cPos = chunkGridPos.pos;

	for (int x = cPos.x; x < cPos.x + m_chunkSize3D.x; ++x)
	{
		for (int y = cPos.y; y < cPos.y + m_chunkSize3D.y; ++y)
		{
			size_t mapX = x, mapY = y;
			// make sure x,y in [0..w),[0..h)
			if (mapX < 0 || mapX >= w || mapY < 0 || mapY >= h) continue;

			int columnHeight = m_heightMap[mapY * w + mapX];

			for (int z = cPos.z; z < cPos.z + m_chunkSize3D.z; ++z)
			{
				if (z > columnHeight)
					break;   // no taller tiles
				if (z < m_waterLevel)
					z = m_waterLevel;
				Grid3D gridPos(x, y, z);
				chunkTiles.tiles.emplace_back(spawnTile(gridPos));
			}
		}
	}
}

Entity Scene_Play::spawnTile(Grid3D& chunkPos)
{
	const static int grassLevel = 24;
	const static int snowLevel = 36;

	sf::Vector2i tileTexPos;
	if (chunkPos.z <= m_waterLevel) tileTexPos = sf::Vector2i(0, 10);
	else if (m_waterLevel < chunkPos.z && chunkPos.z <= grassLevel) tileTexPos = sf::Vector2i(9, 0);
	else if (grassLevel < chunkPos.z && chunkPos.z <= snowLevel) tileTexPos = sf::Vector2i(0, 2);
	else if (snowLevel < chunkPos.z) tileTexPos = sf::Vector2i(0, 2);

	tileTexPos.x *= m_gridCellSize.x;
	tileTexPos.y *= m_gridCellSize.y;

	auto tile = m_entityManager.addEntity(m_memoryPool, "tile", "Tile");
	
	tile.add<CTileRenderInfo>(m_memoryPool, Utils::gridToIsometric(chunkPos, m_gridCellSize),
		sf::IntRect(tileTexPos, sf::Vector2i(m_gridCellSize)));
	tile.add<CGridPosition>(m_memoryPool, chunkPos);

	m_tileMap.insert({ chunkPos, tile });
	return tile;
}

void Scene_Play::update()
{
	if (!m_paused)
	{
		spawnChunks();
		despawnChunks();
		m_entityManager.update(m_memoryPool);
		buildVertexArraysForChunks();
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

	static const float RENDER_DIST = 100.0f;
	static const float RENDER_DIST_SQUARED = RENDER_DIST * RENDER_DIST;

	auto& pGridPos = player().get<CGridPosition>(m_memoryPool).pos;
	for (auto& [gridPos, chunk] : m_chunkMap)
	{
		auto& cGridPos = chunk.get<CGridPosition>(m_memoryPool).pos;
		if (pGridPos.distToSquared(cGridPos) > RENDER_DIST_SQUARED) continue;

		auto& chunkVertexArray = chunk.get<CVertexArray>(m_memoryPool).va;
		window.draw(chunkVertexArray, &m_game->assets().getTexture("TexTiles"));
	}

	auto& animation = player().get<CAnimation>(m_memoryPool).animation;
	window.draw(animation.m_sprite);

	window.setView(window.getDefaultView());

	sf::Text pGridPosText(m_game->assets().getFont("FutureMillennium"), pGridPos.toString());
	window.draw(pGridPosText);

	sf::Text cPosText(m_game->assets().getFont("FutureMillennium"),
		Utils::gridToChunkPos(pGridPos, m_chunkSize3D).toString());
	cPosText.setPosition(sf::Vector2f(0, height() * 0.05f));
	window.draw(cPosText);

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
