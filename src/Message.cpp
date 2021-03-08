#include "Message.h"
#include <iostream>
#include "Player.h"
#include "Map.h"
#include "Snapshot.h"

bool processIncoming(const Message &message, Map &map, std::vector<Player> &players, Snapshot &lastState)
{
	bool result = false;
	//Refer to the snapshot to refresh stats and map info
	if (message.type == eStateUpdate)
	{
		lastState.guard.lock();
		for (int i = 0; i < 4; i++)
		{
			players[i].update(lastState.pos[i],
				static_cast<eDirection>(lastState.facing[i]), lastState.hp[i]);
		}
		lastState.guard.unlock();
		map.update(lastState);
	}
	//Interact with an object
	else if (message.playerId == 1)
	{
		for (int i = 0; i < 4; i++)
		{
			if (players[i].id == message.port && players[i].hp > 0)
			{
				players[i].objectInteract(map, message.pos);
			}
		}
	}
	//Do something player related
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (players[i].id == message.playerId)
			{
				switch (message.type)
				{
				case eMoveOrder:
					if (players[i].hp > 0)
						players[i].move(map, players, message.pos, static_cast<eDirection>(message.stat));
					break;
				case eInteractOrder:
					if (players[i].hp > 0)
						players[i].damage(message.stat);
					break;
				case eDisconnect:
					players[i].damage(INT_MAX);
					result = true;
					break;
				}
				break;
			}
		}

	}
	return result;
}

Message processOutgoing(eMessageType type, unsigned short port, eDirection dir,
	Map &map, std::vector<Player> &players)
{
	int i;
	Message message(type, port);
	//Find correct character
	for (i = 0; i < 4; i++)
	{
		if (players[i].id == port)
			break;
	}
	switch (type)
	{
	case eMoveOrder:
		message.playerId = players[i].id;
		message.pos = players[i].findMovePos(map, dir);
		message.stat = static_cast<int>(dir);
		break;
	case eInteractOrder:
		sf::Vector2i position = sf::Vector2i(0,0);
		message.playerId = players[i].findActionResult(
			map, players, position, dir);
		message.pos = position;
		message.stat = 1;
		break;
	}
	return message;
}

sf::Packet& operator <<(sf::Packet& packet, const Message& m)
{
	return packet << m.type << m.port << m.playerId << m.pos.x << m.pos.y << m.stat;
}

sf::Packet& operator >>(sf::Packet& packet, Message& m)
{
	int tempNum;
	packet >> tempNum >> m.port >> m.playerId >> m.pos.x >> m.pos.y >> m.stat;
	m.type = static_cast<eMessageType>(tempNum);
	return packet;
}

sf::Packet & operator<<(sf::Packet & packet, const Snapshot & m)
{
	for (int i = 0; i < 4; i++)
	{
		packet << m.pos[i].x << m.pos[i].y << m.facing[i] << m.hp[i] << m.spawns[i].x << m.spawns[i].y;
	}
	for (int i = 0; i < 442; i++)
	{
		packet << m.data[i];
	}
	return packet;
}

sf::Packet & operator>>(sf::Packet & packet, Snapshot & m)
{
	for (int i = 0; i < 4; i++)
	{
		packet >> m.pos[i].x >> m.pos[i].y >> m.facing[i] >> m.hp[i] >> m.spawns[i].x >> m.spawns[i].y;
	}
	for (int i = 0; i < 442; i++)
	{
		packet >> m.data[i];
	}
	return packet;
}
