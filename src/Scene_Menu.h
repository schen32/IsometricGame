#pragma once

#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <deque>

#include "EntityManager.hpp"

class Scene_Menu : public Scene
{
protected:
	std::string m_musicName;
	Vec2f m_mousePos;

	void init();
	void loadMenu();
	void update();
	void onEnd();
	void sDoAction(const Action& action);
	void onExitScene();
	void onEnterScene();

	void select();
	void sHover();
	void sAnimation();
public:
	Scene_Menu() = default;
	Scene_Menu(GameEngine* gameEngine = nullptr);
	void sRender();
};