#include "GameEngine.h"
#include "Assets.hpp"
#include "Scene_Menu.h"
#include "Scene_Play.h"

#include <fstream>
#include <iostream>

GameEngine::GameEngine(const std::string& path)
{
	init(path);
}

void GameEngine::init(const std::string& path)
{
	m_assets.loadFromFile(path);

	auto videoMode = sf::VideoMode({ 1920, 1080 });
	m_window.create(videoMode, "Game Engine", sf::Style::Default);
	m_window.setFramerateLimit(60);

	/*if (!ImGui::SFML::Init(m_window))
	{
		std::cerr << "Could not open window." << std::endl;
	}*/

	changeScene("MENU", std::make_shared<Scene_Menu>(this));
}

std::shared_ptr<Scene> GameEngine::currentScene()
{
	return m_sceneMap[m_currentScene];
}

bool GameEngine::isRunning()
{
	return m_running && m_window.isOpen();
}

sf::RenderWindow& GameEngine::window()
{
	return m_window;
}

void GameEngine::run()
{
	while (isRunning())
	{
		if (!m_nextScene.empty())
		{
			if (!m_currentScene.empty())
				currentScene()->onExitScene();

			m_currentScene = m_nextScene;
			m_nextScene.clear();

			currentScene()->onEnterScene();
		}

		//ImGui::SFML::Update(m_window, m_deltaClock.restart());
		update();
	}
	//ImGui::SFML::Shutdown();
	m_window.close();
	
}

void GameEngine::sUserInput()
{
	while (const std::optional event = m_window.pollEvent())
	{
		//ImGui::SFML::ProcessEvent(m_window, *event);

		if (event->is<sf::Event::Closed>())
		{
			quit();
		}

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (currentScene()->getKeyActionMap().find(keyPressed->scancode) ==
				currentScene()->getKeyActionMap().end())
			{
				continue;
			}
			currentScene()->doAction
			(
				Action
				(
				currentScene()->getKeyActionMap().at(keyPressed->scancode),
				"START"
				)
			);
		}

		if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			if (currentScene()->getKeyActionMap().find(keyReleased->scancode) ==
				currentScene()->getKeyActionMap().end())
			{
				continue;
			}
			currentScene()->doAction
			(
				Action
				(
					currentScene()->getKeyActionMap().at(keyReleased->scancode),
					"END"
				)
			);
		}

		if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (currentScene()->getMouseActionMap().find(mousePressed->button) ==
				currentScene()->getMouseActionMap().end())
			{
				continue;
			}
			currentScene()->doAction
			(
				Action
				(
					currentScene()->getMouseActionMap().at(mousePressed->button),
					"START", mousePressed->position
				)
			);
		}

		if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
		{
			if (currentScene()->getMouseActionMap().find(mouseReleased->button) ==
				currentScene()->getMouseActionMap().end())
			{
				continue;
			}
			currentScene()->doAction
			(
				Action
				(
					currentScene()->getMouseActionMap().at(mouseReleased->button),
					"END", mouseReleased->position
				)
			);
		}

		if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
		{
			currentScene()->doAction(Action("MOUSE_MOVE", "START", mouseMoved->position));
		}
	}
}

bool GameEngine::changeScene(const std::string& sceneName, std::shared_ptr<Scene> scene,
	bool endCurrentScene)
{
	if (scene)
	{
		m_sceneMap[sceneName] = scene;
	}
	else if (m_sceneMap.find(sceneName) == m_sceneMap.end())
	{
		std::cerr << "Warning: Scene does not exist: " << sceneName << std::endl;
		return false;
	}

	if (endCurrentScene)
	{
		m_sceneMap.erase(m_sceneMap.find(m_currentScene));
	}

	m_nextScene = sceneName;
	return true;
}

void GameEngine::quit()
{
	m_running = false;
}

const Assets& GameEngine::assets() const
{
	return m_assets;
}

Assets& GameEngine::assets()
{
	return m_assets;
}

void GameEngine::update()
{
	if (!isRunning()) return;
	if (m_sceneMap.empty()) return;

	sUserInput();
	currentScene()->simulate(m_simulationSpeed);
	currentScene()->sRender();

	//ImGui::SFML::Render(m_window);
	m_window.display();
}