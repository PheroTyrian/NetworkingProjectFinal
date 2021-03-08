#ifndef _CLIENT_SYSTEM
#define _CLIENT_SYSTEM

#include <SFML/System.hpp>
#include <string>
#include "CircularBuffer.h"
#include "Global.h"

struct Snapshot;

class ClientSystem
{
public:
	ClientSystem(sf::IpAddress IP, std::string password, Snapshot &snapshot);
	~ClientSystem();

	void messageLoop();
	unsigned short getPort() const;

	CircularBuffer incomingBuffer;
	CircularBuffer outgoingBuffer;
	bool runThread;
private:
	sf::IpAddress ip;
	unsigned short port;
	sf::TcpSocket socket;
	Snapshot & snap;
};

#endif
