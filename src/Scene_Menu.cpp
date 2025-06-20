#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

void Scene_Menu::init()
{
	auto& window = m_game->window();
	window.setView(window.getDefaultView());

	registerMouseAction(sf::Mouse::Button::Left, "LEFT_CLICK");
	registerMouseAction(sf::Mouse::Button::Right, "RIGHT_CLICK");

	loadMenu();
}

void Scene_Menu::loadMenu()
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Game Engine");
	auto& tAnimation = title.add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	auto& tTransform = title.add<CTransform>(Vec2f(width() / 2, height() * 0.15f));
	tTransform.scale = Vec2f(2.f, 1.2f);

	auto playButton = m_entityManager.addEntity("button", "Start");
	playButton.add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& pbTransform = playButton.add<CTransform>(Vec2f(width() / 2, height() * 0.4f));
	playButton.add<CState>("unselected");

	auto quitButton = m_entityManager.addEntity("button", "Quit");
	quitButton.add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& qTransform = quitButton.add<CTransform>(Vec2f(width() / 2, height() * 0.6f));
	quitButton.add<CState>("unselected");
}

void Scene_Menu::update()
{
	m_entityManager.update();
	sHover();
	sAnimation();
}

void Scene_Menu::sHover()
{
	for (Entity button : m_entityManager.getEntities("button"))
	{
		auto& buttonState = button.get<CState>().state;
		if (Utils::isInside(m_mousePos, button))
			buttonState = "selected";
		else
			buttonState = "unselected";
	}
}

void Scene_Menu::sAnimation()
{
	for (Entity button : m_entityManager.getEntities("button"))
	{
		auto& buttonState = button.get<CState>().state;
		auto& buttonAnimation = button.get<CAnimation>().animation;

		if (buttonState == "selected" && buttonAnimation.m_name != "ButtonHover")
		{
			button.add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true);
			playSound("BubblierStep", 15);
		}
		else if (buttonState == "unselected" && buttonAnimation.m_name != "Button")
		{
			button.add<CAnimation>(m_game->assets().getAnimation("Button"), true);
		}
	}
}

void Scene_Menu::onEnd()
{
	m_game->quit();
}

void Scene_Menu::onExitScene()
{

}

void Scene_Menu::onEnterScene()
{
	auto& window = m_game->window();
	window.setView(window.getDefaultView());
}

void Scene_Menu::select()
{
	for (Entity button : m_entityManager.getEntities("button"))
	{
		if (!Utils::isInside(m_mousePos, button)) continue;

		if (button.name() == "Start")
			m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, "assets/play.txt"));
		else if (button.name() == "Quit")
			onEnd();
	}
}

void Scene_Menu::sDoAction(const Action& action)
{
	if (action.m_type == "START")
	{
		if (action.m_name == "QUIT")
		{
			onEnd();
		}
		else if (action.m_name == "MOUSE_MOVE")
			m_mousePos = action.m_mousePos;
		else if (action.m_name == "LEFT_CLICK")
		{
			m_mousePos = action.m_mousePos;
			select();
		}
	}
}

void Scene_Menu::sRender()
{
    auto& window = m_game->window();
	window.clear(sf::Color(204, 226, 225));
    
	for (Entity entity : m_entityManager.getEntities())
	{
		if (!entity.has<CAnimation>()) continue;

		auto& animation = entity.get<CAnimation>().animation;
		auto& transform = entity.get<CTransform>();

		animation.m_sprite.setPosition(transform.pos);
		animation.m_sprite.setScale(transform.scale);
		window.draw(animation.m_sprite);

		auto buttonText = sf::Text(m_game->assets().getFont("FutureMillennium"));

		if (entity.tag() == "ui")
			buttonText.setCharacterSize(150);
		else if (entity.tag() == "button")
			buttonText.setCharacterSize(100);

		buttonText.setString(entity.name());
		buttonText.setOutlineThickness(2.0f);
		buttonText.setOutlineColor(sf::Color(86, 106, 137));
		auto bounds = buttonText.getLocalBounds();
		buttonText.setOrigin(bounds.size / 2.f);
		buttonText.setPosition(transform.pos - bounds.position);
		window.draw(buttonText);
	}
}