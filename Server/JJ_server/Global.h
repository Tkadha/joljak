#pragma once

enum class E_PACKET
{
	E_P_CHAT = 0
};

class PACKET
{
public:
	unsigned char size;
	char type;
	PACKET() : size(sizeof(PACKET)) {}
};

class CHAT_PACKET : public PACKET
{
public:
	char chat[20];
	CHAT_PACKET() {
		size = sizeof(CHAT_PACKET);
	}
};