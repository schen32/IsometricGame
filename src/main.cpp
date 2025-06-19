#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "GameEngine.h"

int main()
{
    GameEngine g("assets/assets.txt");
    g.run();
}