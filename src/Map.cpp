#include "Map.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Snapshot.h"

Map::Map()
{
	//Load textures
	floorTex.loadFromFile("textures/floor.png");
	wallTex.loadFromFile("textures/wall.png");
	doorTex.loadFromFile("textures/door.png");
	openDoorTex.loadFromFile("textures/openDoor.png");
	//Load sprites
	floor.setTexture(floorTex);
	wall.setTexture(wallTex);
	door.setTexture(doorTex);
	openDoor.setTexture(openDoorTex);
}

Map::~Map()
{
}

void Map::draw(sf::RenderWindow &window)
{
	for (int i = 0; i < 442; i++)
	{
		float yPos = 16 * std::floor(i / mapWidth);
		float xPos = 16 * (i - yPos * mapWidth / 16);
		switch (data[i])
		{
		case 0: //Wall
			wall.setPosition(xPos, yPos);
			window.draw(wall);
			break;
		case 1: //Floor
			floor.setPosition(xPos, yPos);
			window.draw(floor);
			break;
		case 2: //Closed door
			door.setPosition(xPos, yPos);
			window.draw(door);
			break;
		case 3: //Open door
			floor.setPosition(xPos, yPos);
			openDoor.setPosition(xPos, yPos);
			window.draw(floor);
			window.draw(openDoor);
			break;
		}
	}
}

void Map::update(Snapshot &snap)
{
	snap.guard.lock();
	memcpy(spawnPoints, snap.spawns, sizeof(spawnPoints[0]) * 4);
	memcpy(data, snap.data, 442);
	snap.guard.unlock();
}

void Map::saveState(Snapshot & snap)
{
	snap.guard.lock();
	memcpy(snap.spawns, spawnPoints, sizeof(spawnPoints[0]));
	memcpy(snap.data, data, 442);
	snap.guard.unlock();
}

eTileType Map::getTile(sf::Vector2i coordinates) const
{
	eTileType result = eWall;
	int x = std::max(0, std::min(coordinates.x, mapWidth - 1));
	int y = std::max(0, std::min(coordinates.y, 441));
	switch (data[x + y * mapWidth])
	{
	case 1: result = eFloor; break;
	case 2: result = eClosedDoor; break;
	case 3: result = eOpenDoor; break;
	}
	return result;
}

void Map::setTile(sf::Vector2i coordinates, eTileType type)
{
	unsigned char result = 0;
	switch (type)
	{
	case eFloor:		result = 1; break;
	case eClosedDoor:	result = 2; break;
	case eOpenDoor:		result = 3; break;
	}
	if (coordinates.x >= 0 && coordinates.x < mapWidth &&
		coordinates.y >= 0 && coordinates.y < mapWidth)
		data[coordinates.x + coordinates.y * mapWidth] = result;
	std::cout << coordinates.x << coordinates.y << result;
}

sf::Vector2i Map::getSpawn(int num)
{
	sf::Vector2i result{ 0, 0 };
	if (num >= 0 && num < 4)
		result = spawnPoints[num];
	return result;
}
