#include "Player.h"
#include "Map.h"
#include <algorithm>
#include <iostream>
#include <SFML/System.hpp>

Player::Player(unsigned short playerId, sf::Vector2i position, std::string texture) :
	id(playerId), pos(position), facing(eDown), hp(MAX_HEALTH)
{
	playerTex.loadFromFile(texture);
	sprite.setTexture(playerTex);
	sprite.setOrigin({ 8, 8 });
}

Player::~Player()
{
}

void Player::draw(sf::RenderWindow &window)
{
	sprite.setPosition(pos.x * 16 + 8, pos.y * 16 + 8);
	sprite.setRotation(static_cast<int>(facing) * 90);
	window.draw(sprite);
}

void Player::update(sf::Vector2i position, eDirection dir, int health)
{
	pos = position;
	facing = dir;
	hp = health;
}

sf::Vector2i Player::findMovePos(const Map & map, eDirection dir) const
{
	sf::Vector2i finalPos = pos;
	switch (dir)
	{
	case eUp:
		finalPos.y--; break;
	case eRight:
		finalPos.x++; break;
	case eDown:
		finalPos.y++; break;
	case eLeft:
		finalPos.x--; break;
	}
	return finalPos;
}

unsigned short Player::findActionResult(
	const Map & map, const std::vector<Player> &players, sf::Vector2i & positionOfAction, eDirection dir)
{
	unsigned short result = 0;
	positionOfAction = pos;
	switch (dir)
	{
	case eUp:
		positionOfAction.y--; break;
	case eRight:
		positionOfAction.x++; break;
	case eDown:
		positionOfAction.y++; break;
	case eLeft:
		positionOfAction.x--; break;
	}
	for (auto &it : players)
	{
		if (it.pos.x == positionOfAction.x && it.pos.y == positionOfAction.y)
		{
			result = it.id;
			break;
		}
	}
	if (result == 0 &&
		(map.getTile(positionOfAction) == eClosedDoor || map.getTile(positionOfAction) == eOpenDoor))
		result = 1;
	return result;
}

void Player::move(const Map & map, const std::vector<Player> &players, sf::Vector2i position, eDirection dir)
{
	bool collision = false;
	if (map.getTile(position) == eWall || map.getTile(position) == eClosedDoor)
		collision = true;
	else
	{
		for (auto &it : players)
		{
			if (it.pos == position)
			{
				collision = true;
				break;
			}
		}
	}
	if (!collision)
		pos = position;
	facing = dir;
	//std::cout << "Position " << pos.x << " " << pos.y << " " << facing << "\n";
}

void Player::damage(int amount)
{
	hp = std::max(0, std::min(MAX_HEALTH, hp - amount));
	if (hp == 0)
		pos = { -1, -1 };
}

void Player::objectInteract(Map & map, sf::Vector2i position)
{
	switch (map.getTile(position))
	{
	case eClosedDoor: map.setTile(position, eOpenDoor); break;
	case eOpenDoor: map.setTile(position, eClosedDoor); break;
	}
}
