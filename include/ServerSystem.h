#ifndef _SERVER_SYSTEM
#define _SERVER_SYSTEM

#include <SFML/Network.hpp>
#include <atomic>
#include <string>
#include "Global.h"
#include "ClientHandler.h"
#include "CircularBuffer.h"

struct Snapshot;

class ServerSystem
{
public:
	ServerSystem(std::string pass, Snapshot &snapshot);
	~ServerSystem();

	void listenLoop();
	bool clientsConnected();
	unsigned short clientPort(int num);
	bool deleteClient(const unsigned short & handlerPort);

	CircularBuffer* incomingBuffer;
	CircularBuffer* outgoingBuffer[4];
	bool runThread;
private:
	const std::string password;
	Snapshot &snap;
	sf::Mutex clientGuard;
	ClientHandler* clientList[4];
	sf::Thread* clientThread[4];
	sf::UdpSocket udpSocket;
};

#endif
