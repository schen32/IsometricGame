#pragma once

#include "Scene.h"
#include <map>
#include <memory>
#include <set>

#include "EntityManager.hpp"
#include "ParticleSystem.hpp"

class Scene_Play : public Scene
{
public:
	std::string				 m_levelPath;
	ParticleSystem			 m_particleSystem;
	sf::View				 m_cameraView;
	Vec2f					 m_mousePos;
	bool					 m_playerDied = false;
	std::string				 m_musicName;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);

	void onEnd();
	void onEnterScene();
	void onExitScene();
	void update();
	void spawnPlayer();
	void spawnLevelEntities();

	Entity player();
	void sDoAction(const Action& action);

	void sMovement();
	void sAnimation();
	void sCollision();
	void sCamera();

	Scene_Play() = default;
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath = "");

	void sRender();
};
