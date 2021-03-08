#ifndef _MESSAGE
#define _MESSAGE

#include <SFML/Network.hpp>
#include "Global.h"
#include <vector>

struct Player;
class Map;
struct Snapshot;

enum eMessageType
{
	eNullMessage = -1,
	eStateUpdate = 0,
	eMoveOrder,
	eDodgeOrder,
	eInteractOrder,
	eDisconnect
};

struct Message
{
	//Data about the message
	eMessageType type;
	unsigned short port;
	//Data about the action
	unsigned short playerId;
	sf::Vector2i pos;
	int stat;

	Message() : type(eNullMessage), port(0), playerId(0), pos(), stat(0) {}
	Message(eMessageType eType, unsigned short portId) :
		type(eType), port(portId), playerId(0), pos(), stat(0) {}
	Message(eMessageType eType, unsigned short portId, unsigned short pid, sf::Vector2i position, int statistic) :
		type(eType), port(portId), playerId(pid), pos(position), stat(statistic) { }
};

//Performs any actions necessitated by an incoming message type,
//if it returns true there is a client disconnect and it must be handled based on the system type
bool processIncoming(const Message &message, Map &map, std::vector<Player> &players, Snapshot &lastState);
//Client function that creates a message based on the type of message requested so it's ready for sending
Message processOutgoing(eMessageType type, unsigned short port, eDirection dir,
	Map &map, std::vector<Player> &players);

#define NO_MESSAGE Message()

//Overloads to allow easy packing and unpacking of messages
sf::Packet& operator <<(sf::Packet& packet, const Message& m);

sf::Packet& operator >>(sf::Packet& packet, Message& m);

sf::Packet& operator <<(sf::Packet& packet, const Snapshot& m);

sf::Packet& operator >>(sf::Packet& packet, Snapshot& m);

#endif
