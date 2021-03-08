#ifndef _GLOBAL
#define _GLOBAL

//Both the client and the server must share the same UDP port
const unsigned short RECEIVER_PORT = 5422;
const unsigned short GREETER_PORT = 5423;
//These are the ports used for connecting to clients by the server, only 1 is used by clients
const unsigned short CLIENT_PORTS[4] = { 5424, 5425, 5426, 5427 };


enum eDirection
{
	eUp,
	eRight,
	eDown,
	eLeft
};

#endif
