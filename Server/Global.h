#pragma once


enum class E_PACKET
{
	E_P_UNKNOWN = 0,
	E_P_CHAT = 1,
	E_P_INGAME = 2,
	E_P_CHANGEPORT = 3,
	E_P_INPUT = 4,
	E_P_ROTATE = 5,
	E_P_POSITION = 6,
	E_P_LOGIN = 7,
	E_P_LOGOUT = 8,

	E_DB_REGISTER = 100,
	E_DB_LOGIN = 101,
	E_DB_SUCCESS_FAIL = 102,
};





const int MAX_BUF_SIZE = 1024; // ���� �ִ� ũ��


#pragma pack(push, 1) // 1byte alignment

struct PlayerInput {
	char MoveForward = false;
	char MoveBackward = false;
	char WalkLeft = false;
	char WalkRight = false;
	char Jump = false;
	char Attack = false; // ��: FŰ �Ǵ� ���콺 Ŭ��
	char Interact = false; // ��: EŰ
	char Run = false; // ��: Shift Ű
	// �ʿ�� �ٸ� Ű�� ���콺 �Է� �߰�

	
};

class FLOAT3
{
public:
	float x, y, z;
	FLOAT3() = default;
	FLOAT3(float x, float y, float z) : x(x), y(y), z(z) {}
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

class INPUT_PACKET : public PACKET
{
public:
	PlayerInput inputData;
	ULONGLONG uid;
	INPUT_PACKET() {
		size = sizeof(INPUT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_INPUT);
	}
};

class ROTATE_PACKET : public PACKET
{	
	public:
		FLOAT3 right;
		FLOAT3 up;
		FLOAT3 look;
		ULONGLONG uid;
		ROTATE_PACKET() {
		size = sizeof(ROTATE_PACKET);
		type = static_cast<char>(E_PACKET::E_P_ROTATE);
	}
};
class POSITION_PACKET : public PACKET
{
	public:
	FLOAT3 position;
	ULONGLONG uid;
	POSITION_PACKET() {
		size = sizeof(POSITION_PACKET);
		type = static_cast<char>(E_PACKET::E_P_POSITION);
	}
};

class LOGIN_PACKET : public PACKET
{
	public:
	ULONGLONG uid;
	LOGIN_PACKET() {
		size = sizeof(LOGIN_PACKET);
		type = static_cast<char>(E_PACKET::E_P_LOGIN);
	}
};
class LOGOUT_PACKET : public PACKET
{
	public:
	ULONGLONG uid;
	LOGOUT_PACKET() {
		size = sizeof(LOGOUT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_LOGOUT);
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
	char result;	//1: ����, 0: ����
	ULONGLONG uid;
	DB_SUCCESS_FAIL_PACKET() {
		size = sizeof(DB_SUCCESS_FAIL_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_SUCCESS_FAIL);
	}
};


#pragma pack(pop)