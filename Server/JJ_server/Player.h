#pragma once
#include "../ServerLib/RemoteClient.h"
#include "stdafx.h"

#include "../Global.h"

// client ����

class Terrain;

class PlayerClient : public RemoteClient
{
public:
	static unordered_map<PlayerClient*, shared_ptr<PlayerClient>> PlayerClients;

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
	PlayerInput					m_lastReceivedInput{}; // ���������� ���� �Է� ���� ����
	ServerPlayerState			m_currentState = ServerPlayerState::Idle; // ���� �� �÷��̾� ���� (ServerPlayerState enum ���� �ʿ�)

	LPVOID						m_pPlayerUpdatedContext = NULL;
	std::shared_ptr<Terrain>	m_pTerrain = nullptr;

	float m_walkSpeed = 50.0f;
	float m_runSpeed = 80.0f;

public:
	PlayerClient() : RemoteClient()
	{
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
		m_Position = XMFLOAT3(1500.f, 0.0f, 1500.f);	// �׽�Ʈ�� ���� ����
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

	DWORD GetDirection() const { return m_direction; }
	void SetDirection(DWORD nDirection) { m_direction = nDirection; }

	void SetRight(const XMFLOAT3& xmf3Right) { m_Right = xmf3Right; }
	void SetUp(const XMFLOAT3& xmf3Up) { m_Up = xmf3Up; }
	void SetLook(const XMFLOAT3& xmf3Look) { m_Look = xmf3Look; }

	XMFLOAT3 GetRight() const { return m_Right; }
	XMFLOAT3 GetUp() const { return m_Up; }
	XMFLOAT3 GetLook() const { return m_Look; }



	void SetTerrain(std::shared_ptr<Terrain> pTerrain) { m_pTerrain = pTerrain; }
	void Update(float fTimeElapsed);
	
	// ���¸ӽ� ���� ������Ʈ �׽�Ʈ
	void Update_test(float deltaTime);
	bool CheckIfGrounded();
	void SnapToGround();





	void processInput(PlayerInput input);

	XMFLOAT3 GetLookVector() { return(m_Look); }
	XMFLOAT3 GetUpVector() { return(m_Up); }
	XMFLOAT3 GetRightVector() { return(m_Right); }

};

