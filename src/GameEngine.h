#pragma once

#include "Scene.h"
#include "Assets.hpp"

#include <memory>
#include <unordered_map>
#include <string>

using SceneMap = std::unordered_map<std::string, std::shared_ptr<Scene>>;

class GameEngine
{
protected:
	sf::RenderWindow m_window;
	Assets m_assets;
	std::string m_currentScene = "";
	std::string m_nextScene = "";
	SceneMap m_sceneMap;
	size_t m_simulationSpeed = 1;
	sf::Clock m_deltaClock;
	bool m_running = true;

	void init(const std::string& path);
	void update();
	void sUserInput();
	std::shared_ptr<Scene> currentScene();

public:
	GameEngine(const std::string& path);
	bool changeScene(const std::string& sceneName,
		std::shared_ptr<Scene> scene, bool endCurrentScene = false);

	void quit();
	void run();

	sf::RenderWindow& window();
	const Assets& assets() const;
	Assets& assets();
	bool isRunning();
};