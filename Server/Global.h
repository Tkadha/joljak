#pragma once

enum class E_PACKET
{
	E_P_UNKNOWN = 0,
	E_P_CHAT = 1,
	E_P_INGAME = 2,
	E_P_CHANGEPORT = 3
};

class PACKET
{
public:
	unsigned char size;
	char type;
	PACKET() : size(sizeof(PACKET)),type(static_cast<char>(E_PACKET::E_P_UNKNOWN)) {}
};

class CHAT_PACKET : public PACKET
{
public:
	char chat[20];
	CHAT_PACKET() {
		size = sizeof(CHAT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_CHAT);
	}
};

class INGAME_PACKET : public PACKET
{
public:
	INGAME_PACKET() {
		size = sizeof(INGAME_PACKET);
		type = static_cast<char>(E_PACKET::E_P_INGAME);
	}
};

class CHANGEPORT_PACKET : public PACKET
{
public:
	short port;
	char addr[15];
	CHANGEPORT_PACKET() {
		size = sizeof(CHANGEPORT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_CHANGEPORT);
	}
};