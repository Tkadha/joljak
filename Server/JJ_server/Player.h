#pragma once
#include "../ServerLib/RemoteClient.h"
#include "stdafx.h"


// client Á¤º¸
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

	LPVOID						m_pPlayerUpdatedContext = NULL;

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
		m_fMaxVelocityXZ = 0.0f;
		m_fMaxVelocityY = 0.0f;
		m_fFriction = 0.0f;

		m_pPlayerUpdatedContext = NULL;
	}
	PlayerClient(SocketType socketType) :RemoteClient(socketType) {}
	~PlayerClient() = default;

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	void SetRight(const XMFLOAT3& xmf3Right) { m_Right = xmf3Right; }
	void SetUp(const XMFLOAT3& xmf3Up) { m_Up = xmf3Up; }
	void SetLook(const XMFLOAT3& xmf3Look) { m_Look = xmf3Look; }
	void Update(float fTimeElapsed);

};

