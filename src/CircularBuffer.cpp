#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(int capacity) :
	bufferSize(capacity), front(0), rear(0), count(0), overspill(), guard()
{
	buffer = new Message[bufferSize];
	overspill = new std::queue<Message>;
}

void CircularBuffer::deposit(Message data)
{
	guard.lock();

	if (count == bufferSize)
	{
		overspill->push(data);
	}
	else
	{
		buffer[rear] = data;
		rear = (rear + 1) % bufferSize;
		++count;
	}
	guard.unlock();
}

Message CircularBuffer::fetch()
{
	guard.lock();

	Message result = NO_MESSAGE;

	if (!overspill->empty())
	{
		result = overspill->front();
		overspill->pop();
	}
	else if (count != 0)
	{
		result = buffer[front];
		front = (front + 1) % bufferSize;
		--count;
	}
	guard.unlock();
	return result;
}

bool CircularBuffer::empty()
{
	bool result = false;
	guard.lock();
	if (count == 0)
		result = true;
	guard.unlock();
	return result;
}
