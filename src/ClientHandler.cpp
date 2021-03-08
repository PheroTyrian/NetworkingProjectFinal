#include "ClientHandler.h"
#include <ctime>
#include <iostream>
#include "Message.h"

ClientHandler::ClientHandler(sf::IpAddress address, const unsigned short portNum,
	CircularBuffer& inBuffer, CircularBuffer& outBuffer, Snapshot &snapshot) :
	runThread(true), port(portNum), incomingBuffer(inBuffer), outgoingBuffer(outBuffer), snap(snapshot)
{
}

ClientHandler::~ClientHandler()
{
	socket.disconnect();
}

void ClientHandler::messageLoop()
{
	bool success{ false };
	{
		sf::TcpListener listener;
		//listener.setBlocking(false);
		//std::clock_t start = std::clock();
		//Spend 10 seconds listening for connection
		//while (static_cast<double>((std::clock() - start) / CLOCKS_PER_SEC) < 10.0)
		{
			listener.listen(port);
			if (listener.accept(socket) == sf::Socket::Done)
			{
				success = true;
				std::cout << "New client connected\n";
				//break;
			}
		}
	}
	if (!success)
	{
		std::cout << "Failed to connect to client\n";
		runThread = false;
	}

	sf::Packet packet;
	Message message;
	while (runThread)
	{
		//Send message, even if empty
		Message message = outgoingBuffer.fetch();
		packet << message;
		if (socket.send(packet) != sf::Socket::Done)
		{
			std::cout << "Client disconnected or sending failed\n";
			runThread = false;
			break;
		}
		//std::cout << "Sent message to client\n";
		packet.clear();
		//If a state update message is being sent send the large followup:
		if (message.type == eStateUpdate)
		{
			packet << snap;
			if (socket.send(packet) != sf::Socket::Done)
			{
				std::cout << "Client disconnected or sending failed\n";
				runThread = false;
				break;
			}
			packet.clear();
		}
		//Receive message
		if (socket.receive(packet) != sf::Socket::Done)
		{
			std::cout << "Client disconnected or receiving failed\n";
			runThread = false;
			break;
		}
		//std::cout << "received message from client\n";
		packet >> message;
		if (message.type != eNullMessage)
			incomingBuffer.deposit(message);
		packet.clear();
	}
	incomingBuffer.deposit(Message(eDisconnect, port));
}
