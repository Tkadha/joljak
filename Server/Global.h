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

	E_O_ADD = 9,
	E_O_REMOVE = 10,
	E_O_CHANGEANIMATION = 11,
	E_O_MOVE = 12,
	E_O_HIT = 13,
	E_O_INVINCIBLE = 14,
	E_O_SETHP = 15,
	E_O_SETOBB = 16,

	E_P_SETHP = 17,
	E_P_CHANGE_STAT = 18,

	E_STRUCT_OBJ = 19,
	E_SYNC_TIME = 20,

	E_GAME_START = 21,
	E_GAME_END = 22,
	E_GAME_NEW = 23,

	E_P_RESPAWN = 24,


	E_DB_REGISTER = 100,
	E_DB_LOGIN = 101,
	E_DB_SUCCESS_FAIL = 102,
};

enum class OBJECT_TYPE
{
	OB_UNKNOWN = 0,
	OB_PLAYER = 1,
	OB_TREE = 2,
	OB_STONE = 3,
	OB_PIG = 4,
	OB_COW = 5,
	OB_WOLF = 6,
	OB_TOAD = 7,
	OB_WASP = 8,
	OB_BAT = 9,
	OB_SNAKE = 10,
	OB_TURTLE = 11,
	OB_SNAIL = 12,
	OB_SPIDER = 13,
	OB_RAPTOR = 14,
	OB_GOLEM = 15,
	ST_WOODWALL = 16,
	ST_FURNACE = 17
};

enum class ANIMATION_TYPE
{
	UNKNOWN = 0,
	IDLE,
	WALK,
	RUN,
	DIE,
	ATTACK,
	HIT,
	SPECIAL_ATTACK,
};


enum class E_STAT
{
	STAMINA = 0, // 스태미나
	HUNGER = 1, // 허기
	THIRST = 2, // 갈증
	HP = 3, // 체력
	MAX_STAMINA = 4, // 최대 스태미나
	MAX_HP = 5, // 최대 체력
	SPEED = 6, // 이동 속도
	NONE = 7 // 상태 없음
};
const int MAX_BUF_SIZE = 1024; // 버퍼 최대 크기


#pragma pack(push, 1) // 1byte alignment

struct PlayerInput {
	char MoveForward = false;
	char MoveBackward = false;
	char WalkLeft = false;
	char WalkRight = false;
	char Jump = false;
	char Attack = false; // 예: F키 또는 마우스 클릭
	char Interact = false; // 예: E키
	char Run = false; // 예: Shift 키
	// 필요시 다른 키나 마우스 입력 추가

	void clear()
	{
		MoveForward = false;
		MoveBackward = false;
		WalkLeft = false;
		WalkRight = false;
		Jump = false;
		Attack = false; 
		Interact = false;
		Run = false; 
	}
};

class FLOAT3
{
public:
	float x, y, z;
	FLOAT3() = default;
	FLOAT3(float x, float y, float z) : x(x), y(y), z(z) {}
};
class FLOAT4
{
public:
	float x, y, z, w;
	FLOAT4() = default;
	FLOAT4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

class PACKET
{
public:
	unsigned char size;
	char type;
	PACKET() : size(sizeof(PACKET)), type(static_cast<char>(E_PACKET::E_P_UNKNOWN)) {}
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
	char addr[15] {};
	CHANGEPORT_PACKET() {
		port = -1;
		size = sizeof(CHANGEPORT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_CHANGEPORT);
	}
};

class INPUT_PACKET : public PACKET
{
public:
	PlayerInput inputData {};
	unsigned long long uid;
	INPUT_PACKET() {
		inputData.clear();
		uid = -1;
		size = sizeof(INPUT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_INPUT);
	}
};

class ROTATE_PACKET : public PACKET
{
public:
	FLOAT3 right {};
	FLOAT3 up {};
	FLOAT3 look {};
	unsigned long long uid;
	ROTATE_PACKET() {
		uid = -1;
		size = sizeof(ROTATE_PACKET);
		type = static_cast<char>(E_PACKET::E_P_ROTATE);
	}
};
class POSITION_PACKET : public PACKET
{
public:
	FLOAT3 position {};
	unsigned long long uid;
	POSITION_PACKET() {
		uid = -1;
		size = sizeof(POSITION_PACKET);
		type = static_cast<char>(E_PACKET::E_P_POSITION);
	}
};

class SET_HP_HIT_OBJ_PACKET : public PACKET
{
public:
	int hit_obj_id;
	int hp;
	SET_HP_HIT_OBJ_PACKET() {
		hit_obj_id = -1;
		hp = 0;
		size = sizeof(SET_HP_HIT_OBJ_PACKET);
		type = static_cast<char>(E_PACKET::E_P_SETHP);
	}
};

class CHANGE_STAT_PACKET : public PACKET
{
public:
	int oid; // 객체 ID
	E_STAT stat; // 상태 (예: 0: 일반, 1: 공격, 2: 방어 등)
	float value;
	CHANGE_STAT_PACKET() {
		oid = -1;
		stat = E_STAT::NONE;
		value = -1;
		size = sizeof(CHANGE_STAT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
	}
};

class ADD_PACKET : public PACKET 
{
public:
	FLOAT3 right{};
	FLOAT3 up{};
	FLOAT3 look{};
	FLOAT3 position{};
	int id;
	OBJECT_TYPE o_type;
	ANIMATION_TYPE a_type;
	ADD_PACKET() {
		id = -1;
		o_type = OBJECT_TYPE::OB_UNKNOWN;
		a_type = ANIMATION_TYPE::UNKNOWN;
		size = sizeof(ADD_PACKET);
		type = static_cast<char>(E_PACKET::E_O_ADD);
	}
};
class REMOVE_PACKET : public PACKET
{
public:
	int id;
	REMOVE_PACKET() {
		id = -1;
		size = sizeof(REMOVE_PACKET);
		type = static_cast<char>(E_PACKET::E_O_REMOVE);
	}
};
class CHANGEANIMATION_PACKET : public PACKET
{
public:
	int oid;
	ANIMATION_TYPE a_type;
	CHANGEANIMATION_PACKET() {
		a_type = ANIMATION_TYPE::UNKNOWN;
		size = sizeof(CHANGEANIMATION_PACKET);
		type = static_cast<char>(E_PACKET::E_O_CHANGEANIMATION);
	}
};
class MOVE_PACKET : public PACKET
{
public:
	FLOAT3 right{};
	FLOAT3 up{};
	FLOAT3 look{};
	FLOAT3 position{};
	int id;
	MOVE_PACKET() {
		size = sizeof(MOVE_PACKET);
		type = static_cast<char>(E_PACKET::E_O_MOVE);
	}
};

class OBJ_HIT_PACKET : public PACKET
{
public:
	int oid;
	int damage;
	OBJ_HIT_PACKET() {
		oid = -1;
		damage = 0;
		size = sizeof(OBJ_HIT_PACKET);
		type = static_cast<char>(E_PACKET::E_O_HIT);
	}
};

class OBJ_HP_PACKET : public PACKET
{
public:
	int oid;
	int hp;
	OBJ_HP_PACKET() {
		oid = -1;
		hp = 0;
		size = sizeof(OBJ_HP_PACKET);
		type = static_cast<char>(E_PACKET::E_O_SETHP);
	}
};

class OBJ_INVINCIBLE_PACKET : public PACKET
{
	public:
	int oid;
	char invincible; // 1: 무적, 0: 무적 해제
	OBJ_INVINCIBLE_PACKET() {
		oid = -1;
		invincible = 0;
		size = sizeof(OBJ_INVINCIBLE_PACKET);
		type = static_cast<char>(E_PACKET::E_O_INVINCIBLE);
	}
};

class OBJ_OBB_PACKET : public PACKET
{
public:
	int oid;
	FLOAT3 Center{};
	FLOAT3 Extents{};
	FLOAT4 Orientation{};
	OBJ_OBB_PACKET() {
		oid = -1;
		size = sizeof(OBJ_OBB_PACKET);
		type = static_cast<char>(E_PACKET::E_O_SETOBB);
	}
};


class STRUCT_OBJ_PACKET : public PACKET
{
public:
	OBJECT_TYPE o_type;
	FLOAT3 Center{};
	FLOAT3 Extents{};
	FLOAT4 Orientation{};
	FLOAT3 right{};
	FLOAT3 up{};
	FLOAT3 look{};
	FLOAT3 position{};
	STRUCT_OBJ_PACKET() {
		o_type = OBJECT_TYPE::OB_UNKNOWN;
		size = sizeof(STRUCT_OBJ_PACKET);
		type = static_cast<char>(E_PACKET::E_STRUCT_OBJ);
	}
};

class TIME_SYNC_PACKET : public PACKET
{
public:
	float serverTime;
	TIME_SYNC_PACKET() {
		serverTime = 0.0f;
		size = sizeof(TIME_SYNC_PACKET);
		type = static_cast<char>(E_PACKET::E_SYNC_TIME); // 시간 동기화는 별도의 타입으로 정의할 수 있음
	}
};

class GAME_START_PACKET:public PACKET
{
public:
	GAME_START_PACKET() {
		type = static_cast<char>(E_PACKET::E_GAME_START);
		size = sizeof(GAME_START_PACKET);
	}
};
class GAME_END_PACKET :public PACKET
{
public:
	GAME_END_PACKET() {
		type = static_cast<char>(E_PACKET::E_GAME_END);
		size = sizeof(GAME_END_PACKET);
	}
};
class NEW_GAME_PACKET :public PACKET
{
public:
	NEW_GAME_PACKET() {
		type = static_cast<char>(E_PACKET::E_GAME_NEW);
		size = sizeof(NEW_GAME_PACKET);
	}
};

class PLAYER_RESPAWN_PACKET : public PACKET
{
public:
	PLAYER_RESPAWN_PACKET() {
		type = static_cast<char>(E_PACKET::E_P_RESPAWN);
		size = sizeof(PLAYER_RESPAWN_PACKET);
	}
};

class LOGIN_PACKET : public PACKET
{
public:
	unsigned long long uid;
	LOGIN_PACKET() {
		uid = -1;
		size = sizeof(LOGIN_PACKET);
		type = static_cast<char>(E_PACKET::E_P_LOGIN);
	}
};
class LOGOUT_PACKET : public PACKET
{
public:
	unsigned long long uid;
	LOGOUT_PACKET() {
		uid = -1;
		size = sizeof(LOGOUT_PACKET);
		type = static_cast<char>(E_PACKET::E_P_LOGOUT);
	}
};
class DB_REGISTER_PACKET : public PACKET
{
public:
	char id[20]{};
	char pw[20]{};
	unsigned long long uid;
	DB_REGISTER_PACKET() {
		uid = -1;
		size = sizeof(DB_REGISTER_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_REGISTER);
	}
};
class DB_LOGIN_PACKET : public PACKET
{
public:
	char id[20]{};
	char pw[20]{};
	unsigned long long uid;
	DB_LOGIN_PACKET() {
		uid = -1;
		size = sizeof(DB_LOGIN_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_LOGIN);
	}
};
class DB_SUCCESS_FAIL_PACKET : public PACKET
{
public:
	char kind{};	// E_DB_REGISTER, E_DB_LOGIN
	char result{};	//1: 성공, 0: 실패
	unsigned long long uid;
	DB_SUCCESS_FAIL_PACKET() {
		uid = -1;
		size = sizeof(DB_SUCCESS_FAIL_PACKET);
		type = static_cast<char>(E_PACKET::E_DB_SUCCESS_FAIL);
	}
};


#pragma pack(pop)