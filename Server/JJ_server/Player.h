#pragma once
#include "../ServerLib/RemoteClient.h"
#include "stdafx.h"
#include "../Global.h"
#include <unordered_set>
#include "Terrain.h"
#include <iostream>
#include <atomic>
// client 정보
#include <mutex>
extern std::mutex g_clients_mutex;

class GameObject;

enum C_STATE { PC_FREE, PC_INGAME };

class PlayerClient : public RemoteClient
{
public:
	static unordered_map<PlayerClient*, shared_ptr<PlayerClient>> PlayerClients;
	std::mutex c_mu;
	C_STATE state;
	std::mutex vl_mu;
	std::unordered_set<int> viewlist;

	BoundingOrientedBox local_obb;
	BoundingOrientedBox world_obb;
private:

	XMFLOAT3					m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	DWORD						m_direction = 0;
	PlayerInput					m_lastReceivedInput{}; // 마지막으로 받은 입력 상태 저장
	ServerPlayerState			m_currentState = ServerPlayerState::Idle; // 서버 측 플레이어 상태 (ServerPlayerState enum 정의 필요)

	LPVOID						m_pPlayerUpdatedContext = NULL;


	float m_walkSpeed = 50.0f;
	float m_runSpeed = 100.0f;
	int Speed_stat = 0;
	bool b_slow = false;

public:
	std::atomic_int Playerhp = 300;
	std::atomic_int Maxhp = 300;
	std::atomic_int Playerstamina = 150;
	std::atomic_int Maxstamina = 150;
	std::atomic<float> PlayerHunger = 100.0f;
	std::atomic<float> PlayerThirst = 100.0f;

	int stamina_counter = 0;
public:
	PlayerClient() : RemoteClient()
	{
		state = PC_FREE;

		m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fMaxVelocityXZ = 100.0f;
		m_fMaxVelocityY = 400.0f;
		m_fFriction = 250.0f;

		m_direction = 0;
		m_currentState = ServerPlayerState::Idle;
		m_pPlayerUpdatedContext = NULL;
	}
	PlayerClient(SocketType socketType) :RemoteClient(socketType) 
	{
		state = PC_FREE;
		m_Position = XMFLOAT3(8000.f, 0.0f, 8000.f);	// 테스트용 임의 지정
		m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Gravity = XMFLOAT3(0.0f, -250.0f, 0.0f);
		m_fMaxVelocityXZ = 100.f;
		m_fMaxVelocityY = 400.0f;
		m_fFriction = 250.0f;

		m_direction = 0;
		m_currentState = ServerPlayerState::Idle;
		m_pPlayerUpdatedContext = NULL;
	}
	~PlayerClient() = default;

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	XMFLOAT3 GetPosition() const { return m_Position; }
	const XMFLOAT3& GetVelocity() const { return(m_Velocity); }
	void SetVelocity(const XMFLOAT3& Velocity) { m_Velocity = Velocity; }
	void SetPosition(const XMFLOAT3& Position) { Move(XMFLOAT3(Position.x - m_Position.x, Position.y - m_Position.y, Position.z - m_Position.z), false); }
	void UpdateTransform();

	DWORD GetDirection() const { return m_direction; }
	void SetDirection(DWORD nDirection) { m_direction = nDirection; }

	void SetRight(const XMFLOAT3& xmf3Right) { m_Right = xmf3Right; }
	void SetUp(const XMFLOAT3& xmf3Up) { m_Up = xmf3Up; }
	void SetLook(const XMFLOAT3& xmf3Look) { m_Look = xmf3Look; }

	XMFLOAT3 GetRight() const { return m_Right; }
	XMFLOAT3 GetUp() const { return m_Up; }
	XMFLOAT3 GetLook() const { return m_Look; }

	void Update(float fTimeElapsed);
	
	// 상태머신 적용 업데이트 테스트
	void Update_test(float deltaTime);
	bool CheckIfGrounded();
	void SnapToGround();

	void SetEffect(OBJECT_TYPE obj_type);
	void SetSlow(bool b) { b_slow = b; }

	void Change_Stat(E_STAT stat, float value);
	ServerPlayerState GetCurrentState() const { return m_currentState; }
	void processInput(PlayerInput input);

	XMFLOAT3 GetLookVector() { return(m_Look); }
	XMFLOAT3 GetUpVector() { return(m_Up); }
	XMFLOAT3 GetRightVector() { return(m_Right); }

	void ResetState()
	{
		m_Position = XMFLOAT3(8000.f, 0.0f, 8000.f);	// 테스트용 임의 지정
		m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Gravity = XMFLOAT3(0.0f, -250.0f, 0.0f);
		m_fMaxVelocityXZ = 100.f;
		m_fMaxVelocityY = 400.0f;
		m_fFriction = 250.0f;

		m_direction = 0;
		m_currentState = ServerPlayerState::Idle;
		m_pPlayerUpdatedContext = NULL;

		Playerhp = 300;
		Maxhp = 300;
		Playerstamina = 150;
		Maxstamina = 150;
		PlayerHunger = 100.0f;
		PlayerThirst = 100.0f;
		Speed_stat = 0;
	}
	void RespawnPlayer()
	{
		m_Position = XMFLOAT3(8000.f, Terrain::terrain->GetHeight(8000.f, 8000.f), 8000.f);	// 테스트용 임의 지정
		m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Playerhp.store(Maxhp.load());
		Playerstamina.store(Maxstamina.load());
		PlayerHunger.store(100.0f);
		PlayerThirst.store(100.0f);
	}
public:
	void BroadCastPosPacket();
	void BroadCastRotatePacket();
	void BroadCastInputPacket();
	void BroadCastHitPacket(PlayerInput pi);
	void SendHpPacket(int,int);
	void SendInvinciblePacket(int, bool);
	void SendAddPacket(shared_ptr<GameObject>);
	void SendRemovePacket(shared_ptr<GameObject>);
	void SendMovePacket(shared_ptr<GameObject>);
	void SendAnimationPacket(shared_ptr<GameObject>);
	void SendStructPacket(shared_ptr<GameObject>);
	void SendTimePacket(float);
	void SendStartGamePacket();
	void SendEndGamePacket();
	void SendNewGamePacket();
};

