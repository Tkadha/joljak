#pragma once

enum class E_PACKET
{
	E_P_UNKNOWN = 0,
	E_P_CHAT = 1,
	E_P_INGAME = 2,
	E_P_CHANGEPORT = 3,

	E_DB_REGISTER = 4,
	E_DB_LOGIN = 5,
	E_DB_SUCCESS_FAIL = 6,
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

class DB_REGISTER_PACKET : public PACKET
{
	public:
	char id[20];
	char pw[20];
	ULONGLONG uid;
	DB_REGISTER_PACKET() {
		size = sizeof(DB_REGISTER_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_REGISTER);
	}
};

class DB_LOGIN_PACKET : public PACKET
{
public:
	char id[20];
	char pw[20];
	ULONGLONG uid;
	DB_LOGIN_PACKET() {
		size = sizeof(DB_LOGIN_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_LOGIN);
	}
};

class DB_SUCCESS_FAIL_PACKET : public PACKET
{
	public:
	char kind;	// E_DB_REGISTER, E_DB_LOGIN
	char result;	//1: 성공, 0: 실패
	ULONGLONG uid;
	DB_SUCCESS_FAIL_PACKET() {
		size = sizeof(DB_SUCCESS_FAIL_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_SUCCESS_FAIL);
	}
};
