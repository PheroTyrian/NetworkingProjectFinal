#ifndef _CIRCULAR_BUFFER
#define _CIRCULAR_BUFFER

#include <SFML/Network.hpp>
#include <queue>
#include "Message.h"

class CircularBuffer
{
public:
	CircularBuffer(int capacity);
	~CircularBuffer() { delete[] buffer; delete overspill; }

	void deposit(Message data);
	Message fetch();
	bool empty();
private:
	Message* buffer;
	const int bufferSize;
	int front;
	int rear;
	int count;
	sf::Mutex guard;
	std::queue<Message>* overspill;
};

#endif
