#include "stdafx.h"
#include "Player.h"
#include "Terrain.h"
#include "GameObject.h"
#include "Octree.h"
#include "Event.h"
#include <iostream>

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#define MIN_HEIGHT                  1055.f      
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

void PlayerClient::UpdateTransform()
{
    XMVECTOR vRight = XMLoadFloat3(&m_Right);
    XMVECTOR vUp = XMLoadFloat3(&m_Up);
    XMVECTOR vLook = XMLoadFloat3(&m_Look);
    XMVECTOR vPosition = XMLoadFloat3(&m_Position);
    XMFLOAT4X4 xmf4x4 = Matrix4x4::Identity();
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._11), vRight);
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._21), vUp);
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._31), vLook);
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._41), vPosition);

    XMMATRIX worldMatrix = XMLoadFloat4x4(&xmf4x4);



    XMVECTOR localCenter = XMLoadFloat3(&local_obb.Center);
    XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, worldMatrix);
    XMStoreFloat3(&world_obb.Center, worldCenter);


    XMMATRIX rotationMatrix = worldMatrix;
    rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR orientation = XMQuaternionRotationMatrix(rotationMatrix);
    XMStoreFloat4(&world_obb.Orientation, orientation);


    XMFLOAT3 scale;
    scale.x = XMVectorGetX(XMVector3Length(worldMatrix.r[0]));
    scale.y = XMVectorGetX(XMVector3Length(worldMatrix.r[1]));
    scale.z = XMVectorGetX(XMVector3Length(worldMatrix.r[2]));
    world_obb.Extents.x = local_obb.Extents.x * scale.x;
    world_obb.Extents.y = local_obb.Extents.y * scale.y;
    world_obb.Extents.z = local_obb.Extents.z * scale.z;
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

	if(Terrain::terrain)
	{		
		XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
		bool bReverseQuad = ((z % 2) != 0);
		float fHeight = Terrain::terrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
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

void PlayerClient::Change_Stat(E_STAT stat, float value)
{
    if (this->state != PC_INGAME) return;
	switch (stat)
	{
	case E_STAT::STAMINA:
        Playerstamina.store(value);
		break;
    case E_STAT::HUNGER: {
        float expectedHunger = PlayerHunger.load();
        while (true) {
            float desiredHunger = value;

            if (desiredHunger > 100.f) {
                desiredHunger = 100.f;
            }
            if (PlayerHunger.compare_exchange_weak(expectedHunger, desiredHunger)) {
                break;
            }
        }
    }
		break;
    case E_STAT::THIRST: {
        float expectedThirst = PlayerThirst.load();
        while (true) {
            float desiredThirst = value;

            if (desiredThirst > 100.f) {
                desiredThirst = 100.f;
            }
            if (PlayerThirst.compare_exchange_weak(expectedThirst, desiredThirst)) {
                break;
            }
        }
    }
		break;
	case E_STAT::MAX_STAMINA:
        Maxstamina.store(value);
		break;
	case E_STAT::HP:
        Playerhp.store(value);
		break;
	case E_STAT::MAX_HP:
        Maxhp.store(value);
		break;
	case E_STAT::SPEED:
        Speed_stat = value; // 이동 속도
		break;
	default:
		break;
	}

}

void PlayerClient::processInput(PlayerInput input)
{
	m_lastReceivedInput = input;
}

void PlayerClient::Update_test(float deltaTime)
{
    if (this->state != PC_INGAME) return;
    PlayerInput currentInput = m_lastReceivedInput;
    // 공격 상태 처리
    static float attackTimer = 0.0f; // 실제로는 멤버 변수로 관리
    if (m_currentState == ServerPlayerState::Attacking) {
        attackTimer -= deltaTime;
        if (attackTimer <= 0.0f) {
            m_currentState = ServerPlayerState::Idle; // 공격 끝나면 Idle로
        }
    }

    // 상태전환
    bool isGrounded = CheckIfGrounded();
    if (m_currentState != ServerPlayerState::Attacking) { // 공격 중 아닐 때만
        if (currentInput.Attack) {
            m_currentState = ServerPlayerState::Attacking;
            attackTimer = 0.8f; // 공격 지속 시간
            m_Velocity.x = m_Velocity.z = 0; // 공격 중 이동 불가
        }
        else if (currentInput.Jump && isGrounded) {
            m_currentState = ServerPlayerState::Jumping;
            m_Velocity.y = 150.0f; // 점프 초기 속도
        }
        else if (isGrounded && m_currentState == ServerPlayerState::Falling) { // 착지
            m_currentState = ServerPlayerState::Idle; // 착지하면 Idle
            m_Velocity.y = 0; 
        }
        else if (m_Velocity.y < -1.0f && !isGrounded && m_currentState != ServerPlayerState::Jumping) { // 떨어지는 중 
            m_currentState = ServerPlayerState::Falling;
        }
        else if (isGrounded && (currentInput.MoveForward || currentInput.MoveBackward || currentInput.WalkLeft || currentInput.WalkRight)) {
            // 땅에 있고 이동 입력이 있으면 Walking 또는 Running
            if (currentInput.Run) m_currentState = ServerPlayerState::Running;
            else m_currentState = ServerPlayerState::Walking;
           // m_currentState = (currentInput.Run && currentInput.MoveForward) ? ServerPlayerState::Running : ServerPlayerState::Walking;
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
        if (currentInput.WalkLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMovingInput = true;
        }
        if (currentInput.WalkRight) {
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
    //XMFLOAT3 deltaPos = Vector3::ScalarProduct(m_Velocity, deltaTime);
    XMFLOAT3 deltaVel = Vector3::ScalarProduct(m_Velocity, 0.75f);

    if (Speed_stat > 0) {
        deltaVel.x *= (1.f + 0.1f * Speed_stat);
        deltaVel.z *= (1.f + 0.1f * Speed_stat);
    }

    // 런닝 판정
    if (m_currentState == ServerPlayerState::Running)
    {
        deltaVel.x *= 1.5f;
        deltaVel.z *= 1.5f;

    }
    // 슬로우 효과 확인
    if (b_slow)
    {
        deltaVel.x /= 1.75f;
        deltaVel.z /= 1.75f;
    }


    /* 충돌 처리*/
    // 이동 충돌처리 적용
    // X축 이동 시도
    XMFLOAT3 moving_pos = m_Position;
    moving_pos.x += deltaVel.x;
    BoundingOrientedBox testOBBX;
    XMMATRIX matX;
    if (m_Velocity.y == 0)
        matX = XMMatrixTranslation(moving_pos.x, Terrain::terrain->GetHeight(moving_pos.x, moving_pos.z), moving_pos.z);
    else
        matX = XMMatrixTranslation(moving_pos.x, moving_pos.y, moving_pos.z);
    local_obb.Transform(testOBBX, matX);
    testOBBX.Orientation.w = 1.f;

    // octree 적용해서 범위 줄이기
    std::vector<tree_obj*> presults;
    std::vector<tree_obj*> oresults;
    {
        tree_obj n_obj{ -1 ,moving_pos };
        Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, presults);
        Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, oresults);
        for (auto& p_obj : presults) {
            for (auto& cl : PlayerClient::PlayerClients) {
                if (cl.second->state != PC_INGAME)continue;
                if (cl.second->m_id != p_obj->u_id) continue;
                if (testOBBX.Intersects(cl.second->world_obb))
                {
                    moving_pos.x = m_Position.x;
                    break;
                }
            }
        }
        for (auto& o_obj : oresults) {
            if (GameObject::gameObjects[o_obj->u_id]->GetID() < 0) continue;
            if (GameObject::gameObjects[o_obj->u_id]->_hp <= 0) continue;
            if (false == GameObject::gameObjects[o_obj->u_id]->is_alive) continue;
            if (testOBBX.Intersects(GameObject::gameObjects[o_obj->u_id]->world_obb))
            {
                moving_pos.x = m_Position.x;
                break;
            }
        }
        for (auto& c_obj : GameObject::ConstructObjects) {
            if (testOBBX.Intersects(c_obj->world_obb))
            {
                moving_pos.x = m_Position.x;
                break;
            }
        }
    }

    // Z축 이동 시도
    moving_pos.z += deltaVel.z;
    BoundingOrientedBox testOBBZ;
    XMMATRIX matZ;
    if (m_Velocity.y == 0)
        matZ = XMMatrixTranslation(moving_pos.x, Terrain::terrain->GetHeight(moving_pos.x, moving_pos.z), moving_pos.z);
    else
        matZ = XMMatrixTranslation(moving_pos.x, moving_pos.y, moving_pos.z); local_obb.Transform(testOBBZ, matZ);
    testOBBZ.Orientation.w = 1.f;

    presults.clear();
    oresults.clear();
    {
        tree_obj n_obj{ -1 ,moving_pos };
        Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, presults);
        Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, oresults);
        for (auto& p_obj : presults) {
            for (auto& cl : PlayerClient::PlayerClients) {
                if (cl.second->state != PC_INGAME)continue;
                if (cl.second->m_id != p_obj->u_id) continue;
                if (testOBBZ.Intersects(cl.second->world_obb))
                {
                    moving_pos.z = m_Position.z;
                    break;
                }
            }
        }
        for (auto& o_obj : oresults) {
            if (GameObject::gameObjects[o_obj->u_id]->GetID() < 0) continue;
            if (GameObject::gameObjects[o_obj->u_id]->_hp <= 0) continue;
            if (false == GameObject::gameObjects[o_obj->u_id]->is_alive) continue;
            if (testOBBZ.Intersects(GameObject::gameObjects[o_obj->u_id]->world_obb))
            {
                moving_pos.z = m_Position.z;
                break;
            }
        }
        for (auto& c_obj : GameObject::ConstructObjects) {
            if (testOBBZ.Intersects(c_obj->world_obb))
            {
                moving_pos.z = m_Position.z;
                break;
            }
        }
    }


    XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
    XMFLOAT3 xmf3PlayerPosition = moving_pos;
    int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
    bool bReverseQuad = ((z % 2) != 0);
    FLOAT move_pos_y = Terrain::terrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
    if (move_pos_y < MIN_HEIGHT) {
        moving_pos = m_Position;
    }
    else
        moving_pos.y += deltaVel.y;

    SetPosition(moving_pos);
    // 땅 짚기
    SnapToGround();
}

bool PlayerClient::CheckIfGrounded()
{
    if (this->state != PC_INGAME) return false;
    XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
    XMFLOAT3 xmf3PlayerPosition = GetPosition();
    int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
    bool bReverseQuad = ((z % 2) != 0);
    float fHeight = Terrain::terrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
    if (xmf3PlayerPosition.y <= fHeight) return true;
    
    return false;
}

void PlayerClient::SnapToGround()
{
    if (this->state != PC_INGAME) return;
    XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
    XMFLOAT3 xmf3PlayerPosition = GetPosition();
    int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
    bool bReverseQuad = ((z % 2) != 0);
    float fHeight = Terrain::terrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;

    if (xmf3PlayerPosition.y < fHeight) {
        XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
        xmf3PlayerVelocity.y = 0.0f;
        SetVelocity(xmf3PlayerVelocity);

        xmf3PlayerPosition.y = fHeight;
        SetPosition(xmf3PlayerPosition);
    }
    // 건축물이 생길 시 제거후 로직 변경하기
    if (m_currentState != ServerPlayerState::Jumping) {
        xmf3PlayerPosition.y = fHeight;
        SetPosition(xmf3PlayerPosition);
    }
}

void PlayerClient::SetEffect(OBJECT_TYPE obj_type)
{
    switch (obj_type)
    {
    case OBJECT_TYPE::OB_TOAD:
    {
        SetSlow(true);
        EVENT ev{ EVENT_TYPE::E_P_SLOW_END, m_id, -1 };
        EVENT::add_timer(ev, 5000); // 5초 후에 슬로우 효과 제거
    }
    break;
    case OBJECT_TYPE::OB_SPIDER:
    {
        EVENT ev{ EVENT_TYPE::E_P_POISON, m_id, -1 };
        ev.end_time = std::chrono::system_clock::now() + std::chrono::milliseconds(1000) * 10;
        EVENT::add_timer(ev, 500);
    }
    break;
    case OBJECT_TYPE::OB_RAPTOR:
    case OBJECT_TYPE::OB_WOLF:
    {
        EVENT ev{ EVENT_TYPE::E_P_BLEEDING, m_id, -1 };
        ev.end_time = std::chrono::system_clock::now() + std::chrono::milliseconds(1000) * 10;
        EVENT::add_timer(ev, 1000);
    }
    break;
    default:
        break;
    }
}

void PlayerClient::BroadCastPosPacket()
{
    POSITION_PACKET s_packet;
    s_packet.size = sizeof(POSITION_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_P_POSITION);
    s_packet.uid = m_id;
    s_packet.position.x = m_Position.x;
    s_packet.position.y = m_Position.y;
    s_packet.position.z = m_Position.z;

    for (auto& client : PlayerClient::PlayerClients) {
        if (client.second->state != PC_INGAME) continue;
        client.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
    }
}

void PlayerClient::BroadCastRotatePacket()
{
    ROTATE_PACKET s_packet;
    s_packet.size = sizeof(ROTATE_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_P_ROTATE);
    s_packet.right = FLOAT3{ m_Right.x,m_Right.y,m_Right.z };
    s_packet.up = FLOAT3{ m_Up.x,m_Up.y,m_Up.z };
    s_packet.look = FLOAT3{ m_Look.x,m_Look.y,m_Look.z };
    s_packet.uid = m_id;
    for (auto& cl : PlayerClient::PlayerClients) {
        if (cl.second->state != PC_INGAME) continue;
        if (cl.second->m_id == m_id) continue; // 나 자신은 제외한다.
        cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
    }
}

void PlayerClient::BroadCastInputPacket()
{
    INPUT_PACKET s_packet;
    s_packet.size = sizeof(INPUT_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_P_INPUT);
    s_packet.inputData = m_lastReceivedInput;
    s_packet.uid = m_id;
    for (auto& cl : PlayerClient::PlayerClients) {
        if (cl.second->state != PC_INGAME) continue;
        if (cl.second->m_id == m_id) continue; // 나 자신은 제외한다.
        cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
    }
}

void PlayerClient::SendHpPacket(int oid, int hp)
{
    OBJ_HP_PACKET s_packet;
    s_packet.size = sizeof(OBJ_HP_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_O_SETHP);
    s_packet.oid = oid;
    s_packet.hp = hp;
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendInvinciblePacket(int oid, bool invin_type)
{
	OBJ_INVINCIBLE_PACKET s_packet;
	s_packet.size = sizeof(OBJ_INVINCIBLE_PACKET);
	s_packet.type = static_cast<char>(E_PACKET::E_O_INVINCIBLE);
	s_packet.oid = oid;
	s_packet.invincible = invin_type ? 1 : 0;
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendAddPacket(shared_ptr<GameObject> obj)
{
    if (false == obj->is_alive) return; // 리스폰 중 상태라면
    vl_mu.lock();
    viewlist.insert(obj->GetID());
    vl_mu.unlock();

    ADD_PACKET s_packet;
    s_packet.size = sizeof(ADD_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_O_ADD);
    s_packet.position.x = obj->GetPosition().x;
    s_packet.position.y = obj->GetPosition().y;
    s_packet.position.z = obj->GetPosition().z;
    s_packet.right.x = obj->GetNonNormalizeRight().x;
    s_packet.right.y = obj->GetNonNormalizeRight().y;
    s_packet.right.z = obj->GetNonNormalizeRight().z;
    s_packet.up.x = obj->GetNonNormalizeUp().x;
    s_packet.up.y = obj->GetNonNormalizeUp().y;
    s_packet.up.z = obj->GetNonNormalizeUp().z;
    s_packet.look.x = obj->GetNonNormalizeLook().x;
    s_packet.look.y = obj->GetNonNormalizeLook().y;
    s_packet.look.z = obj->GetNonNormalizeLook().z;
    s_packet.o_type = obj->GetType();
    s_packet.a_type = obj->GetAnimationType();
    s_packet.id = obj->GetID();
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendRemovePacket(shared_ptr<GameObject> obj)
{
    vl_mu.lock();
    viewlist.erase(obj->GetID());
    vl_mu.unlock();

    REMOVE_PACKET s_packet;
    s_packet.size = sizeof(REMOVE_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_O_REMOVE);
    s_packet.id = obj->GetID();
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendMovePacket(shared_ptr<GameObject> obj)
{
    MOVE_PACKET s_packet;
    s_packet.size = sizeof(MOVE_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_O_MOVE);
    s_packet.position.x = obj->GetPosition().x;
    s_packet.position.y = obj->GetPosition().y;
    s_packet.position.z = obj->GetPosition().z;
    s_packet.right.x = obj->GetNonNormalizeRight().x;
    s_packet.right.y = obj->GetNonNormalizeRight().y;
    s_packet.right.z = obj->GetNonNormalizeRight().z;
    s_packet.up.x = obj->GetNonNormalizeUp().x;
    s_packet.up.y = obj->GetNonNormalizeUp().y;
    s_packet.up.z = obj->GetNonNormalizeUp().z;
    s_packet.look.x = obj->GetNonNormalizeLook().x;
    s_packet.look.y = obj->GetNonNormalizeLook().y;
    s_packet.look.z = obj->GetNonNormalizeLook().z;
    s_packet.id = obj->GetID();
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendAnimationPacket(shared_ptr<GameObject> obj)
{
    CHANGEANIMATION_PACKET s_packet;
    s_packet.size = sizeof(CHANGEANIMATION_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_O_CHANGEANIMATION);
    s_packet.oid = obj->GetID();
    s_packet.a_type = obj->GetAnimationType();
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendStructPacket(shared_ptr<GameObject> obj)
{
    STRUCT_OBJ_PACKET s_packet;
    s_packet.size = sizeof(STRUCT_OBJ_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_STRUCT_OBJ);
    s_packet.o_type = obj->GetType();
    s_packet.position.x = obj->GetPosition().x;
    s_packet.position.y = obj->GetPosition().y;
    s_packet.position.z = obj->GetPosition().z;
    s_packet.right.x = obj->GetNonNormalizeRight().x;
    s_packet.right.y = obj->GetNonNormalizeRight().y;
    s_packet.right.z = obj->GetNonNormalizeRight().z;
    s_packet.up.x = obj->GetNonNormalizeUp().x;
    s_packet.up.y = obj->GetNonNormalizeUp().y;
    s_packet.up.z = obj->GetNonNormalizeUp().z;
    s_packet.look.x = obj->GetNonNormalizeLook().x;
    s_packet.look.y = obj->GetNonNormalizeLook().y;
    s_packet.look.z = obj->GetNonNormalizeLook().z;
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}

void PlayerClient::SendTimePacket(float t)
{
    TIME_SYNC_PACKET s_packet;
    s_packet.size = sizeof(TIME_SYNC_PACKET);
    s_packet.type = static_cast<char>(E_PACKET::E_SYNC_TIME);
    s_packet.serverTime = t;
    tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
}
