#include "Scene.h"
#include "GameEngine.h"

Scene::Scene(GameEngine* gameEngine)
	: m_game(gameEngine) { }

void Scene::setPaused(bool paused)
{
	m_paused = paused;
}

size_t Scene::width() const
{
	return m_game->window().getSize().x;
}

size_t Scene::height() const
{
	return m_game->window().getSize().y;
}

size_t Scene::currentFrame() const
{
	return m_currentFrame;
}

const KeyActionMap& Scene::getKeyActionMap() const
{
	return m_keyActionMap;
}

const MouseActionMap& Scene::getMouseActionMap() const
{
	return m_mouseActionMap;
}

void Scene::registerKeyAction(sf::Keyboard::Scan keyCode, const std::string& name)
{
	m_keyActionMap[keyCode] = name;
}

void Scene::registerMouseAction(sf::Mouse::Button mouseButton, const std::string& name)
{
	m_mouseActionMap[mouseButton] = name;
}

bool Scene::hasEnded() const
{
	return m_hasEnded;
}

void Scene::simulate(const size_t frames)
{
	for (size_t i = 0; i < frames; i++)
	{
		update();
	}
	m_currentFrame++;
}

void Scene::doAction(const Action& action)
{
	sDoAction(action);
}

void Scene::playSound(const std::string& name, float volume)
{
	auto& sound = m_game->assets().getSound(name);
	sound.setVolume(volume);
	sound.play();
}

void Scene::playVariablePitchSound(const std::string& name, float volume)
{
	auto& sound = m_game->assets().getSound(name);
	float pitch = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f; // range [0.8, 1.2]
	sound.setPitch(pitch);
	sound.setVolume(volume);
	sound.play();
}