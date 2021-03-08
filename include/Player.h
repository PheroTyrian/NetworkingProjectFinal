#ifndef _PLAYER
#define _PLAYER

#include "Global.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

class Map;

const int MAX_HEALTH = 5;

struct Player
{
	unsigned short id;
	sf::Vector2i pos;
	eDirection facing;
	int hp;
	sf::Texture playerTex;
	sf::Sprite sprite;

	Player(unsigned short playerId, sf::Vector2i position, std::string texture);
	~Player();

	void draw(sf::RenderWindow &window);
	//Replaces the player's current stats with those sent in a state update
	void update(sf::Vector2i position, eDirection dir, int health);

	//Returns the result of a client using the move keys
	sf::Vector2i findMovePos(const Map &map, eDirection dir) const;
	//Returns the result of a player using the action button,
	//if it returns 1 then it is an object interaction at 'positionOfAction'
	//if it returns 0 then nothing happens
	unsigned short findActionResult(
		const Map &map, const std::vector<Player> &players, sf::Vector2i &positionOfAction , eDirection dir);

	void move(const Map &map, const std::vector<Player> &players, sf::Vector2i position, eDirection dir);
	void damage(int amount = 1);
	void objectInteract(Map &map, sf::Vector2i position);
};

#endif
