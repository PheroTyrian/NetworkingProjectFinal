#ifndef _SNAPSHOT
#define _SNAPSHOT

#include <SFML/System.hpp>
#include <vector>
#include "Player.h"
#include "Map.h"

struct Snapshot
{
	sf::Mutex guard;
	//Player data
	sf::Vector2i pos[4];
	int facing[4];
	int hp[4];
	//Map data
	sf::Vector2i spawns[4];
	unsigned char data[442];
};

#endif
