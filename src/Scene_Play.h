#pragma once

#include "Scene.h"
#include <map>
#include <memory>
#include <set>

#include "Grid3D.hpp"
#include "EntityManager.hpp"
#include "ParticleSystem.hpp"

using TileMap = std::map<Grid3D, Entity>;
using ChunkMap = std::map<Grid3D, Entity>;
using HeightMap = std::vector<float>;

class Scene_Play : public Scene
{
public:
	std::string				 m_levelPath;
	ParticleSystem			 m_particleSystem;
	sf::View				 m_cameraView;
	Vec2f					 m_mousePos;
	bool					 m_playerDied = false;
	std::string				 m_musicName;
	Vec2f					 m_gridCellSize = { 32, 32 };
	Grid3D   				 m_gridSize3D = { 1000, 1000, 50 };
	Grid3D					 m_chunkSize3D = { 32, 32, 32 };
	Grid3D					 m_numChunks3D = { 4, 4, 4 };
	TileMap					 m_tileMap;
	ChunkMap				 m_chunkMap;
	int						 m_loadRadius = 3;
	bool					 m_chunkChanged = false;
	HeightMap				 m_heightMap;
	int						 m_waterLevel = 20;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);
	void generateHeightMap();

	void onEnd();
	void onEnterScene();
	void onExitScene();
	void update();
	void spawnPlayer();
	Entity spawnChunk(const Grid3D& chunkPos);
	void spawnTilesFromChunk(const CGridPosition& chunkPos, CChunkTiles& chunkTiles);
	void spawnTiles();
	Entity spawnTile(Grid3D& chunkPos);

	Entity player();
	void sDoAction(const Action& action);

	void sMovement();
	void sAI();
	void sStatus();
	void sAnimation();
	void sCollision();
	void sCamera();
	void sGui();
	void sSelect();

	void spawnChunks();
	void despawnChunks();

	Scene_Play() = default;
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath = "");

	void sRender();
	void buildVertexArrayForChunk(CVertexArray& cVa, CChunkTiles& tileChunk, const sf::Texture& tileset);
	void buildVertexArraysForChunks();
};
