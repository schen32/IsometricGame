#pragma once

#include "Animation.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <SFML/Audio.hpp>

class Assets
{
public:
	std::unordered_map<std::string, sf::Texture> m_textureMap;
	std::unordered_map<std::string, Animation> m_animationMap;
	std::unordered_map<std::string, sf::Font> m_fontMap;
	std::unordered_map<std::string, sf::SoundBuffer> m_soundBufferMap;
	std::unordered_map<std::string, sf::Sound> m_soundMap;
	std::unordered_map<std::string, sf::Music> m_musicMap;

	void addTexture(const std::string& textureName, const std::string& path,
		bool smooth = false)
	{
		m_textureMap[textureName] = sf::Texture();

		if (!m_textureMap[textureName].loadFromFile(path))
		{
			std::cerr << "Could not load texture from file: " << path << std::endl;
			return;
		}

		m_textureMap[textureName].setSmooth(smooth);
	}

	void addAnimation(const std::string& animationName, const std::string& textureName,
		size_t rows, size_t cols, size_t frameCount, size_t speed)
	{
		m_animationMap[animationName] = Animation(animationName, m_textureMap[textureName],
			rows, cols, frameCount, speed);
	}

	void addFont(const std::string& fontName, const std::string& path)
	{
		m_fontMap[fontName] = sf::Font();
		if (!m_fontMap[fontName].openFromFile(path))
		{
			std::cerr << "Could not load font file: " << path << std::endl;
		}
	}

	void addSound(const std::string& soundName, const std::string& path)
	{
		m_soundBufferMap[soundName] = sf::SoundBuffer();
		if (!m_soundBufferMap[soundName].loadFromFile(path))
		{
			std::cerr << "Could not load sound file: " << path << std::endl;
		}
		m_soundMap.emplace(soundName, sf::Sound(m_soundBufferMap[soundName]));
	}

	void addMusic(const std::string& musicName, const std::string& path)
	{
		m_musicMap[musicName] = sf::Music();
		if (!m_musicMap[musicName].openFromFile(path))
		{
			std::cerr << "Could not load music file: " << path << std::endl;
		}
	}
	
	Assets() = default;
	void loadFromFile(const std::string& path)
	{
		auto file = std::ifstream(path);
		std::string str;
		while (file.good())
		{
			file >> str;
			if (str == "Texture")
			{
				std::string name, path;
				file >> name >> path;
				addTexture(name, path);
			}
			else if (str == "Animation")
			{
				std::string name, texture;
				size_t rows, cols;
				size_t frames, speed;
				file >> name >> texture >> rows >> cols >> frames >> speed;
				addAnimation(name, texture, rows, cols, frames, speed);
			}
			else if (str == "Font")
			{
				std::string name, path;
				file >> name >> path;
				addFont(name, path);
			}
			else if (str == "Sound")
			{
				std::string name, path;
				file >> name >> path;
				addSound(name, path);
			}
			else if (str == "Music")
			{
				std::string name, path;
				file >> name >> path;
				addMusic(name, path);
			}
			else
			{
				std::cerr << "Unknown Asset Type: " << str << std::endl;
			}
		}
	}

	const sf::Texture& getTexture(const std::string& textureName) const
	{
		assert(m_textureMap.find(textureName) != m_textureMap.end());
		return m_textureMap.at(textureName);
	}

	const Animation& getAnimation(const std::string& animationName) const
	{
		assert(m_animationMap.find(animationName) != m_animationMap.end());
		return m_animationMap.at(animationName);
	}

	const sf::Font& getFont(const std::string& fontName) const
	{
		assert(m_fontMap.find(fontName) != m_fontMap.end());
		return m_fontMap.at(fontName);
	}

	sf::Sound& getSound(const std::string& soundName)
	{
		assert(m_soundMap.find(soundName) != m_soundMap.end());
		return m_soundMap.at(soundName);
	}

	sf::Music& getMusic(const std::string& musicName)
	{
		assert(m_musicMap.find(musicName) != m_musicMap.end());
		return m_musicMap.at(musicName);
	}
};