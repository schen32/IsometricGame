#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.hpp"
#include "ParticleSystem.hpp"

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
	Vec2f					 m_gridSize = { 16, 16 };

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);

	void onEnd();
	void onEnterScene();
	void onExitScene();
	void update();
	void spawnPlayer();
	void spawnTiles(const std::string& filename);
	void spawnTile(float gridX, float gridY, const std::string& aniName);

	std::shared_ptr<Entity> player();
	void sDoAction(const Action& action);
	Vec2f gridToIsometric(float gridX, float gridY, std::shared_ptr<Entity> entity);
	Vec2f isometricToGrid(float posX, float posY);

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
