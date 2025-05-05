#include "stdafx.h"
#include "Player.h"
#include "Terrain.h"
#include <iostream>

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

unordered_map<PlayerClient*, shared_ptr<PlayerClient>> PlayerClient::PlayerClients;

void PlayerClient::Move(ULONG dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void PlayerClient::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_Velocity = Vector3::Add(m_Velocity, xmf3Shift);
	}
	else
	{
		m_Position = Vector3::Add(m_Position, xmf3Shift);
	}
}

void PlayerClient::Update(float fTimeElapsed)
{
	m_Velocity = Vector3::Add(m_Velocity, m_Gravity);
	float fLength = sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_Velocity.x *= (fMaxVelocityXZ / fLength);
		m_Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_Velocity.y * m_Velocity.y);
	if (fLength > m_fMaxVelocityY) m_Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if(m_pTerrain)
	{		
		XMFLOAT3 xmf3Scale = m_pTerrain->GetScale();
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
		bool bReverseQuad = ((z % 2) != 0);
		float fHeight = m_pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
		if (xmf3PlayerPosition.y < fHeight)
		{
			XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
			xmf3PlayerVelocity.y = 0.0f;
			SetVelocity(xmf3PlayerVelocity);
			xmf3PlayerPosition.y = fHeight;
			SetPosition(xmf3PlayerPosition);
		}
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}

	fLength = Vector3::Length(m_Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_Velocity = Vector3::Add(m_Velocity, Vector3::ScalarProduct(m_Velocity, -fDeceleration, true));
}

void PlayerClient::processInput(PlayerInputData input)
{
	m_lastReceivedInput = input;
}

void PlayerClient::Update_test(float deltaTime)
{
    PlayerInputData currentInput;
    {
        currentInput = m_lastReceivedInput;
    }


    // ���� ���� ó��
    static float attackTimer = 0.0f; // �����δ� ��� ������ ����
    if (m_currentState == ServerPlayerState::Attacking) {
        attackTimer -= deltaTime;
        if (attackTimer <= 0.0f) {
            m_currentState = ServerPlayerState::Idle; // ���� ������ Idle��
        }
    }

    // �Է� ó��
    bool isGrounded = CheckIfGrounded();
    if (m_currentState != ServerPlayerState::Attacking) { // ���� �� �ƴ� ����
        if (currentInput.Attack) {
            m_currentState = ServerPlayerState::Attacking;
            attackTimer = 0.8f; // ���� ���� �ð�
            m_Velocity.x = m_Velocity.z = 0; // ���� �� �̵� �Ұ�
        }
        else if (currentInput.Jump && isGrounded) {
            m_currentState = ServerPlayerState::Jumping;
            m_Velocity.y = 300.0f; // ���� �ʱ� �ӵ�
        }
        else if (isGrounded && m_currentState == ServerPlayerState::Falling) { // ����
            m_currentState = ServerPlayerState::Idle; // �����ϸ� Idle
            m_Velocity.y = 0; 
        }
        else if (m_Velocity.y < -1.0f && !isGrounded && m_currentState != ServerPlayerState::Jumping) { // �������� �� 
            m_currentState = ServerPlayerState::Falling;
        }
        else if (isGrounded && (currentInput.MoveForward || currentInput.MoveBackward || currentInput.MoveLeft || currentInput.MoveRight)) {
            // ���� �ְ� �̵� �Է��� ������ Walking �Ǵ� Running
            m_currentState = (currentInput.Run && currentInput.MoveForward) ? ServerPlayerState::Running : ServerPlayerState::Walking;
        }
        else if (isGrounded) {
            // ���� �ְ� �ٸ� ���� ������ Idle
            m_currentState = ServerPlayerState::Idle;
        }
    }


    // �ӵ� ���
    XMFLOAT3 targetVelocityXZ = { 0.0f, 0.0f, 0.0f };
    if (m_currentState == ServerPlayerState::Walking || m_currentState == ServerPlayerState::Running || m_currentState == ServerPlayerState::Falling || m_currentState == ServerPlayerState::Jumping) { 
        XMFLOAT3 moveVector = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 look = GetLookVector(); 
        XMFLOAT3 right = GetRightVector(); 
        bool isMovingInput = false;

        if (currentInput.MoveForward) {
            moveVector = Vector3::Add(moveVector, look);
            isMovingInput = true;
        }
        if (currentInput.MoveBackward) {
            moveVector = Vector3::Add(moveVector, look, -1.0f);
            isMovingInput = true;
        }
        if (currentInput.MoveLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMovingInput = true;
        }
        if (currentInput.MoveRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMovingInput = true;
        }

        if (isMovingInput) {
            moveVector = Vector3::Normalize(moveVector);
            float currentSpeed = (m_currentState == ServerPlayerState::Running) ? m_runSpeed : m_walkSpeed; // ���º� �ӵ�
            targetVelocityXZ = Vector3::ScalarProduct(moveVector, currentSpeed);
        }
    }

    // �ӵ� ����
    m_Velocity.x = targetVelocityXZ.x;
    m_Velocity.z = targetVelocityXZ.z;

    // �߷� ����
    if (!isGrounded || m_currentState == ServerPlayerState::Jumping) {
        m_Velocity.y += m_Gravity.y * deltaTime; // m_gravity.y�� �������� ��
    }

    // ���� ����
    if (isGrounded && m_currentState != ServerPlayerState::Running && m_currentState != ServerPlayerState::Walking) { // Idle ������
        m_Velocity.x *= (1.0f - m_fFriction * deltaTime);
        m_Velocity.z *= (1.0f - m_fFriction * deltaTime);
    }

    // �ӵ� ����
    float fLength = sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
    float fMaxVelocityXZ = m_fMaxVelocityXZ;
    if (fLength > m_fMaxVelocityXZ)
    {
        m_Velocity.x *= (fMaxVelocityXZ / fLength);
        m_Velocity.z *= (fMaxVelocityXZ / fLength);
    }
    float fMaxVelocityY = m_fMaxVelocityY;
    fLength = sqrtf(m_Velocity.y * m_Velocity.y);
    if (fLength > m_fMaxVelocityY) m_Velocity.y *= (fMaxVelocityY / fLength);


    // �̵� �� �浹 ó��
    XMFLOAT3 deltaPos = Vector3::ScalarProduct(m_Velocity, deltaTime);
    /* �浹 ó��*/
    XMFLOAT3 finalDeltaPos = deltaPos; // �浹 ó�� ����

    // ��ġ ������Ʈ
    m_Position = Vector3::Add(m_Position, finalDeltaPos);

    // �� ¤��
    SnapToGround();
}

bool PlayerClient::CheckIfGrounded()
{
    XMFLOAT3 xmf3Scale = m_pTerrain->GetScale();
    XMFLOAT3 xmf3PlayerPosition = GetPosition();
    int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
    bool bReverseQuad = ((z % 2) != 0);
    float fHeight = m_pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
    if (xmf3PlayerPosition.y < fHeight) return true;
    
    return false;
}

void PlayerClient::SnapToGround()
{
    XMFLOAT3 xmf3Scale = m_pTerrain->GetScale();
    XMFLOAT3 xmf3PlayerPosition = GetPosition();
    int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
    bool bReverseQuad = ((z % 2) != 0);
    float fHeight = m_pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;

    if (xmf3PlayerPosition.y < fHeight) {
        XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
        xmf3PlayerVelocity.y = 0.0f;
        SetVelocity(xmf3PlayerVelocity);

        xmf3PlayerPosition.y = fHeight;
        SetPosition(xmf3PlayerPosition);
    }
}