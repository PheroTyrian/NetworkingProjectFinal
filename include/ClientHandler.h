#ifndef _CLIENT_HANDLER
#define _CLIENT_HANDLER

#include <SFML/Network.hpp>
#include "CircularBuffer.h"

class ClientHandler
{
public:
	ClientHandler(sf::IpAddress address, const unsigned short portNum,
		CircularBuffer& inBuffer, CircularBuffer& outBuffer, Snapshot &snapshot);
	~ClientHandler();
	//This is the port the listener will temporarily work on not the one the tcp connection will be on
	const unsigned short port;
	bool runThread;

	//This is the loop executed by an external thread
	void messageLoop();
private:
	sf::TcpSocket socket;
	CircularBuffer& incomingBuffer;
	CircularBuffer& outgoingBuffer;
	Snapshot &snap;
};

#endif
