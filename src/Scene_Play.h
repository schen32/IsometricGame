#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "Grid3D.hpp"
#include "EntityManager.hpp"
#include "ParticleSystem.hpp"

using TileMap = std::map<Grid3D, Entity>;

class Scene_Play : public Scene
{
protected:
	std::string				 m_levelPath;
	ParticleSystem			 m_particleSystem;
	sf::View				 m_cameraView;
	Vec2f					 m_mousePos;
	bool					 m_playerDied = false;
	std::string				 m_musicName;
	Vec2f					 m_gridCellSize = { 32, 32 };
	Grid3D   				 m_gridSize3D = { 100, 100, 20 };
	TileMap					 m_tileMap;
	Entity  m_selectedTile;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);

	void onEnd();
	void onEnterScene();
	void onExitScene();
	void update();
	void spawnPlayer();
	void spawnTiles(const std::string& filename);
	void spawnTile(float gridX, float gridY, float gridZ, const std::string& aniName);

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
public:

	Scene_Play() = default;
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath = "");

	void sRender();
};
