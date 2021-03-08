#include "ServerSystem.h"
#include "ClientHandler.h"
#include <iostream>

ServerSystem::ServerSystem(std::string pass, Snapshot &snapshot) :
	runThread(true), clientList(), clientThread(), snap(snapshot), password(pass)
{
	incomingBuffer = new CircularBuffer(16);

	for (int i = 0; i < 4; i++)
		clientList[i] = nullptr;

	if (udpSocket.bind(RECEIVER_PORT) != sf::Socket::Done)
	{
		std::cout << "Error: ServerSystem could not port " << RECEIVER_PORT << "\n";
		runThread = false;
	}

	udpSocket.setBlocking(false);
}

ServerSystem::~ServerSystem()
{
	udpSocket.unbind();
	clientGuard.lock();
	for (int i = 0; i < 4; i++)
	{
		if (clientList[i])
		{
			clientList[i]->runThread = false;
			clientThread[i]->wait();
			delete clientList[i];
			delete outgoingBuffer[i];
		}
	}
	delete incomingBuffer;
}

bool ServerSystem::deleteClient(const unsigned short& handlerPort)
{
	clientGuard.lock();
	for (int i = 0; i < 4; i++)
	{
		if (clientList[i] && clientList[i]->port == handlerPort)
		{
			clientList[i]->runThread = false;
			clientThread[i]->wait();
			delete clientList[i];
			delete outgoingBuffer[i];
			clientList[i] = nullptr;
			outgoingBuffer[i] = nullptr;
			clientGuard.unlock();
			return true;
		}
	}
	clientGuard.unlock();
	return false;
}

void ServerSystem::listenLoop()
{
	sf::Packet clientData;
	sf::IpAddress arrivalIp;
	unsigned short arrivalPort;

	while (runThread)
	{
		if (udpSocket.receive(clientData, arrivalIp, arrivalPort) == sf::Socket::Done)
		{
			unsigned short clientPort = 0;
			std::cout << "Received greeting from client\n";
			//Password validation
			std::string clientPassCode;
			clientData >> clientPassCode;
			if (password == clientPassCode)
			{
				clientGuard.lock();
				//Find a free slot for the new client
				int i;
				for (i = 0; i < 4; i++)
				{
					if (!clientList[i])
						break;
				}
				outgoingBuffer[i] = new CircularBuffer(8);
				clientList[i] = new ClientHandler(arrivalIp, CLIENT_PORTS[i], *incomingBuffer, *outgoingBuffer[i], snap);
				clientThread[i] = new sf::Thread(&ClientHandler::messageLoop, clientList[i]);
				clientThread[i]->launch();
				clientGuard.unlock();

				clientPort = CLIENT_PORTS[i];
			}
			else std::cout << "Password " << clientPassCode << " rejected\n";
			sf::Packet outgoing;
			//If clientport arrives as 0 the client knows the connection was rejected
			outgoing << clientPort;
			udpSocket.send(outgoing, arrivalIp, arrivalPort);
		}
	}
	std::cout << "Listener has stopped\n";
}

bool ServerSystem::clientsConnected()
{
	bool result = false;
	clientGuard.lock();
	for (int i = 0; i < 4; i++)
	{
		if (clientList[i])
			result = true;
	}
	clientGuard.unlock();
	return result;
}

unsigned short ServerSystem::clientPort(int num)
{
	unsigned short result{ 0 };
	if(clientList[num])
		result = clientList[num]->port;
	return result;
}
