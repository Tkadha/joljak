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


    // 공격 상태 처리
    static float attackTimer = 0.0f; // 실제로는 멤버 변수로 관리
    if (m_currentState == ServerPlayerState::Attacking) {
        attackTimer -= deltaTime;
        if (attackTimer <= 0.0f) {
            m_currentState = ServerPlayerState::Idle; // 공격 끝나면 Idle로
        }
    }

    // 입력 처리
    bool isGrounded = CheckIfGrounded();
    if (m_currentState != ServerPlayerState::Attacking) { // 공격 중 아닐 때만
        if (currentInput.Attack) {
            m_currentState = ServerPlayerState::Attacking;
            attackTimer = 0.8f; // 공격 지속 시간
            m_Velocity.x = m_Velocity.z = 0; // 공격 중 이동 불가
        }
        else if (currentInput.Jump && isGrounded) {
            m_currentState = ServerPlayerState::Jumping;
            m_Velocity.y = 300.0f; // 점프 초기 속도
        }
        else if (isGrounded && m_currentState == ServerPlayerState::Falling) { // 착지
            m_currentState = ServerPlayerState::Idle; // 착지하면 Idle
            m_Velocity.y = 0; 
        }
        else if (m_Velocity.y < -1.0f && !isGrounded && m_currentState != ServerPlayerState::Jumping) { // 떨어지는 중 
            m_currentState = ServerPlayerState::Falling;
        }
        else if (isGrounded && (currentInput.MoveForward || currentInput.MoveBackward || currentInput.MoveLeft || currentInput.MoveRight)) {
            // 땅에 있고 이동 입력이 있으면 Walking 또는 Running
            m_currentState = (currentInput.Run && currentInput.MoveForward) ? ServerPlayerState::Running : ServerPlayerState::Walking;
        }
        else if (isGrounded) {
            // 땅에 있고 다른 조건 없으면 Idle
            m_currentState = ServerPlayerState::Idle;
        }
    }


    // 속도 계산
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
            float currentSpeed = (m_currentState == ServerPlayerState::Running) ? m_runSpeed : m_walkSpeed; // 상태별 속도
            targetVelocityXZ = Vector3::ScalarProduct(moveVector, currentSpeed);
        }
    }

    // 속도 설정
    m_Velocity.x = targetVelocityXZ.x;
    m_Velocity.z = targetVelocityXZ.z;

    // 중력 적용
    if (!isGrounded || m_currentState == ServerPlayerState::Jumping) {
        m_Velocity.y += m_Gravity.y * deltaTime; // m_gravity.y는 음수여야 함
    }

    // 마찰 적용
    if (isGrounded && m_currentState != ServerPlayerState::Running && m_currentState != ServerPlayerState::Walking) { // Idle 에서만
        m_Velocity.x *= (1.0f - m_fFriction * deltaTime);
        m_Velocity.z *= (1.0f - m_fFriction * deltaTime);
    }

    // 속도 제한
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


    // 이동 및 충돌 처리
    XMFLOAT3 deltaPos = Vector3::ScalarProduct(m_Velocity, deltaTime);
    /* 충돌 처리*/
    XMFLOAT3 finalDeltaPos = deltaPos; // 충돌 처리 적용

    // 위치 업데이트
    m_Position = Vector3::Add(m_Position, finalDeltaPos);

    // 땅 짚기
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