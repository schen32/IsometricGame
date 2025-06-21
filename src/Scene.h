#pragma once

#include "Action.hpp"
#include "EntityManager.hpp"
#include "MemoryPool.hpp"

#include <memory>

class GameEngine;

using KeyActionMap = std::unordered_map<sf::Keyboard::Scan, std::string>;
using MouseActionMap = std::unordered_map<sf::Mouse::Button, std::string>;

class Scene
{
protected:
	GameEngine* m_game = nullptr;
	EntityManager m_entityManager;
	MemoryPool m_memoryPool;
	KeyActionMap m_keyActionMap;
	MouseActionMap m_mouseActionMap;
	bool m_paused = false;
	bool m_hasEnded = false;
	size_t m_currentFrame = 0;

	virtual void onEnd() = 0;
	void setPaused(bool paused);

public:
	Scene() = default;
	Scene(GameEngine* gameEngine);

	virtual void update() = 0;
	virtual void sDoAction(const Action& action) = 0;
	virtual void sRender() = 0;
	virtual void onExitScene() = 0;
	virtual void onEnterScene() = 0;

	virtual void doAction(const Action& action);
	void simulate(const size_t frames);
	void registerKeyAction(sf::Keyboard::Scan inputKey, const std::string& actionName);
	void registerMouseAction(sf::Mouse::Button inputButton, const std::string& actionName);

	size_t width() const;
	size_t height() const;
	size_t currentFrame() const;

	bool hasEnded() const;
	const KeyActionMap& getKeyActionMap() const;
	const MouseActionMap& getMouseActionMap() const;

	void playSound(const std::string& name, float volume);
	void playVariablePitchSound(const std::string& name, float volume);
};