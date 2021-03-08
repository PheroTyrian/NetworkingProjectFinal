#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include "ServerSystem.h"
#include "ClientSystem.h"
#include "Snapshot.h"
#include "Player.h"
#include "Map.h"

ServerSystem* establishServer(std::string password, sf::Thread* &thread, Snapshot &snap)
{
	std::cout << "Acting as server\n   Local ip is: " << sf::IpAddress::getLocalAddress()
		<< "\n   Public ip is: " << sf::IpAddress::getPublicAddress(sf::seconds(5)) << "\n";
	ServerSystem* system = new ServerSystem(password, snap);
	thread = new sf::Thread(&ServerSystem::listenLoop, system);
	thread->launch();
	return system;
}

ClientSystem* establishClient(std::string &ip, unsigned short &port, std::string password, sf::Thread* &thread, Snapshot &snap)
{
	sf::IpAddress address = ip.c_str();
	ClientSystem* system = new ClientSystem(address, password, snap);
	port = system->getPort();
	thread = new sf::Thread(&ClientSystem::messageLoop, system);
	thread->launch();
	return system;
}

int main()
{
	unsigned short myPort{ 0 };
	sf::Thread* networkingThread;
	//Only one of these will be used, depending on whether the system runs as client or server
	ServerSystem* server{ nullptr };
	ClientSystem* client{ nullptr };

	Snapshot* snap = new Snapshot;
	Map* map = new Map;
	std::vector<Player> players {
		{CLIENT_PORTS[0], map->getSpawn(0), "textures/player1.png"},
		{CLIENT_PORTS[1], map->getSpawn(1), "textures/player2.png"},
		{CLIENT_PORTS[2], map->getSpawn(2), "textures/player3.png"},
		{CLIENT_PORTS[3], map->getSpawn(3), "textures/player4.png"},
	};

	char choice{ 0 };
	do
	{
		std::cout << "Run as server or client [s/c]? ";
		std::cin >> choice;
	} while (choice != 's' && choice != 'c');
	if (choice == 's')
	{
		std::string password;
		std::cout << "Define server password: ";
		std::cin >> password;
		server = establishServer(password, networkingThread, *snap);
	}
	else
	{
		std::string ipAddress;
		std::cout << "Input server ip: ";
		std::cin >> ipAddress;
		std::string password;
		std::cout << "Input password: ";
		std::cin >> password;
		client = establishClient(ipAddress, myPort, password, networkingThread, *snap);
	}
	Message message;
	if (server)
	{
		bool run{ true };
		bool noConnections{ true };
		std::clock_t timeOut = std::clock();
		std::clock_t snapshotTimer = std::clock();
		std::cout << "Server will close on its own (safely) after 300 seconds of idling without clients,\n"
			<< "or if you hit 'Escape'\n";
		//Do the server loop
		while (run)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				break;
			if (noConnections)
			{
				if (static_cast<double>((std::clock() - timeOut) / CLOCKS_PER_SEC) > 300.0)
					break;
				if (server->clientsConnected())
					noConnections = false;
			}
			//Handle incoming messages
			message = server->incomingBuffer->fetch();
			if (message.type != eNullMessage)
			{
				std::cout << "Message type: " << message.type << "\n";
				if (processIncoming(message, *map, players, *snap))
				{
					//The incoming message was a disconnect message
					if (server->deleteClient(message.port))
						std::cout << "Client handler deleted\n";
					if (!server->clientsConnected())
						run = false;
				}
				//Send incoming message off to all clients except the sender
				else
				{
					for (int i = 0; i < 4; i++)
					{

						if (server->outgoingBuffer[i] && message.port != server->clientPort(i))
							server->outgoingBuffer[i]->deposit(message);
					}
				}
			}
			//If it's time for a snapshot, take one and send it off to clients
			if (static_cast<double>((std::clock() - snapshotTimer) / CLOCKS_PER_SEC) > 5.0)
			{
				snap->guard.lock();
				for (int i = 0; i < 4; i++)
				{
					snap->pos[i] = players[i].pos;
					snap->facing[i] = players[i].facing;
					snap->hp[i] = players[i].hp;
				}
				snap->guard.unlock();
				map->saveState(*snap);
				for (int i = 0; i < 4; i++)
				{
					if (server->outgoingBuffer[i])
						server->outgoingBuffer[i]->deposit(Message(eStateUpdate, 0));
				}
				snapshotTimer = std::clock();
			}
		}
	}
	else if (client)
	{
		//Find the client's player for convenience
		int num;
		for (num = 0; num < 4; num++)
		{
			if (myPort == players[num].id)
				break;
		}
		Player &thisPlayer = players[num];
		//Initialise persistent variables
		std::clock_t lastInputTime = std::clock();
		eDirection moveDir = eUp;
		//Start window
		sf::RenderWindow window(sf::VideoMode(400, 400), "NetworkDemo");
		//Do the client loop
		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}
			//If enough time since last input has elapsed take user input
			if (static_cast<double>((std::clock() - lastInputTime) / CLOCKS_PER_SEC) > 0.2)
			{
				lastInputTime = std::clock();
				//Movement keys
				bool moving = false;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					moveDir = eUp;
					moving = true;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					moveDir = eDown;
					moving = true;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				{
					moveDir = eLeft;
					moving = true;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					moveDir = eRight;
					moving = true;
				}
				//Interact button takes precedence over movement
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
				{
					if (!moving)
						moveDir = thisPlayer.facing;
					message = processOutgoing(eInteractOrder, myPort, moveDir, *map, players);
					client->outgoingBuffer.deposit(message);
					client->incomingBuffer.deposit(message);
				}
				else if (moving)
				{
					message = processOutgoing(eMoveOrder, myPort, moveDir, *map, players);
					client->outgoingBuffer.deposit(message);
					client->incomingBuffer.deposit(message);
				}
			}
			//Read incoming messages and process them
			while (!client->incomingBuffer.empty())
			{
				message = client->incomingBuffer.fetch();
				std::cout << "Processing message type " << message.type << "\n";
				if (message.type != eNullMessage)
				{
					if (processIncoming(message, *map, players, *snap))
						window.close();
				}
			}
			//Draw things relevant to the game
			window.clear();
			map->draw(window);
			for (auto &it : players)
			{
				if (it.hp > 0)
					it.draw(window);
			}
			window.display();
		}
	}

	//Clear up
	if (server)
	{
		std::cout << "Closing server\n";
		server->runThread = false;
	}
	if (client)
	{
		std::cout << "Closing client\n";
		client->runThread = false;
	}
	networkingThread->wait();
	delete server;
	delete client;
	delete snap;
	delete map;
	return 0;
}
