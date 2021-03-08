#include "ClientSystem.h"
#include <iostream>
#include "Message.h"
#include "Snapshot.h"

ClientSystem::ClientSystem(sf::IpAddress IP, std::string password, Snapshot &snapshot) :
	ip(IP), incomingBuffer(8), outgoingBuffer(8), port(0), runThread(true), snap(snapshot)
{
	sf::UdpSocket udpSocket;
	if (udpSocket.bind(GREETER_PORT) != sf::Socket::Done)
	{
		std::cout << "Error: ClientSystem could not bind to port " << GREETER_PORT << "\n";
		runThread = false;
	}
	else
	{
		sf::Packet greeting;
		greeting << password;
		udpSocket.send(greeting, ip, RECEIVER_PORT);
		std::cout << "Sent greeting to server\n";
		sf::IpAddress tempIP;
		unsigned short tempPort;
		sf::Packet reply;
		udpSocket.receive(reply, tempIP, tempPort);
		std::cout << "Received reply from server\n";
		reply >> tempPort;
		if (tempPort != 0)
			port = tempPort;
		else
			std::cout << "Password rejected by server\n";
	}
	udpSocket.unbind();
}

ClientSystem::~ClientSystem()
{
	socket.disconnect();
}

void ClientSystem::messageLoop()
{
	if (port == 0 || socket.connect(ip, port, sf::seconds(10)) != sf::Socket::Done)
	{
		std::cout << "Error: client could not connect to server\n";
		runThread = false;
	}
	sf::Packet packet;
	Message message;
	while (runThread)
	{
		//Receive message
		if (socket.receive(packet) != sf::Socket::Done)
		{
			std::cout << "Server disconnected or receiving failed\n";
			runThread = false;
			break;
		}
		//std::cout << "received message from server\n";
		packet >> message;
		packet.clear();
		//If a larger state update message is on its way:
		if (message.type == eStateUpdate)
		{
			std::cout << "State update received\n";
			if (socket.receive(packet) != sf::Socket::Done)
			{
				std::cout << "Server disconnected or receiving failed\n";
				runThread = false;
				break;
			}
			snap.guard.lock();
			packet >> snap;
			snap.guard.unlock();
			incomingBuffer.deposit(message);
			packet.clear();
		}
		//Otherwise just push the message to buffer
		else if (message.type != eNullMessage)
			incomingBuffer.deposit(message);

		//Send message, even if empty
		Message message = outgoingBuffer.fetch();
		packet << message;
		if (socket.send(packet) != sf::Socket::Done)
		{
			std::cout << "Server disconnected or sending failed\n";
			runThread = false;
			break;
		}
		//std::cout << "sent message to server\n";
		packet.clear();
	}
	incomingBuffer.deposit(Message(eDisconnect, port));
	//TODO:: Make game read dcMessage and then shut down the Game
}

unsigned short ClientSystem::getPort() const
{
	return port;
}
