#include "AtkState.h"
#include "Object.h"
#include "Scene.h"
#include <iostream>
#include "RandomUtil.h"
#include <cmath> // sqrt, pow �Լ� ���

//=====================================Standing=================================================
void AtkNPCStandingState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // ������ �ð�(1~3��)�� �и��ʷ� ��ȯ
	std::cout << "Standing State Enter, duration_time: " << duration_time << std::endl;
	npc->m_pSkinnedAnimationController->SetTrackEnable(0, true);
}

void AtkNPCStandingState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCMoveState>());
		return;
	}

	// �ֺ��� �÷��̾ �ִ��� Ȯ��
	// �÷��̾ ������ Chase�� ����
	if (npc->m_pScene) {
		auto playerInfo = npc->m_pScene->GetPlayerInfo();
		if (playerInfo)
		{
			XMFLOAT3 playerPos = playerInfo->GetPosition();
			XMFLOAT3 npcPos = npc->GetPosition();

			// �� ��ġ ������ 3D �Ÿ� ���
			float distance = sqrt(
				pow(playerPos.x - npcPos.x, 2) +
				pow(playerPos.y - npcPos.y, 2) +
				pow(playerPos.z - npcPos.z, 2)
			);

			// 500 ���� ���� �ִٸ� Chase ���·� ��ȯ
			float detectionRange = 500.f;
			if (distance < detectionRange)
			{
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
				return;
			}
		}
	}

}

void AtkNPCStandingState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->m_pSkinnedAnimationController->SetTrackEnable(0, false);
}




//=====================================Move=================================================

void AtkNPCMoveState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // ������ �ð�(1~3��)�� �и��ʷ� ��ȯ
	move_type = rand_type(dre); // ������ �̵� Ÿ��(0~2)
	rotate_type = rand_type(dre) % 2; // ������ ȸ�� Ÿ��(0~1)
	std::cout << "Move State Enter, duration_time: " << duration_time << std::endl;
	switch (npc->m_objectType)
	{
	case GameObjectType::Spider:
	case GameObjectType::Wasp:
	case GameObjectType::Wolf:
	case GameObjectType::Snake:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(2, true);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}

void AtkNPCMoveState::Execute(std::shared_ptr<CGameObject> npc)
{
	// ������ �̵�
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
		return;
	}
	switch (move_type)
	{
	case 0:
		// ����
		npc->MoveForward(0.2f);
		break;
	case 1:
		// ȸ���ϸ鼭 ����
		npc->Rotate(0.f, 0.5f, 0.f);
		npc->MoveForward(0.1f);
		break;
	case 2:
		// ȸ��
		npc->Rotate(0.f, 0.25f, 0.f);
		break;
	}

	// standing�� �����ϰ� ��ġ ������� ���� ���� ���� chase ���� ����
	if (npc->m_pScene) {
		auto playerInfo = npc->m_pScene->GetPlayerInfo();
		if (playerInfo)
		{
			XMFLOAT3 playerPos = playerInfo->GetPosition();
			XMFLOAT3 npcPos = npc->GetPosition();

			// �� ��ġ ������ 3D �Ÿ� ���
			float distance = sqrt(
				pow(playerPos.x - npcPos.x, 2) +
				pow(playerPos.y - npcPos.y, 2) +
				pow(playerPos.z - npcPos.z, 2)
			);

			// 500 ���� ���� �ִٸ� Chase ���·� ��ȯ
			float detectionRange = 500.f;
			if (distance < detectionRange)
			{
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
				return;
			}
		}
	}
}

void AtkNPCMoveState::Exit(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Spider:
	case GameObjectType::Wasp:
	case GameObjectType::Wolf:
	case GameObjectType::Snake:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(1, false);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}

//=====================================Chase=================================================

void AtkNPCChaseState::Enter(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Spider:
	case GameObjectType::Wasp:
	case GameObjectType::Wolf:
	case GameObjectType::Snake:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(2, true);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}

void AtkNPCChaseState::Execute(std::shared_ptr<CGameObject> npc)
{
	// �÷��̾� ��ġ�� �ٰ���. ���������� �ٰԵǸ� attack ���� ����
	if (npc->m_pScene)
	{
		auto playerInfo = npc->m_pScene->GetPlayerInfo();
		if (playerInfo)
		{
			XMFLOAT3 playerPos = playerInfo->GetPosition();
			XMFLOAT3 npcPos = npc->GetPosition();

			// �÷��̾� ���� ���� ���
			XMVECTOR targetDirectionVec = XMVector3Normalize(XMVectorSet(playerPos.x - npcPos.x, 0.0f, playerPos.z - npcPos.z, 0.0f));
			XMFLOAT3 targetDirection;
			XMStoreFloat3(&targetDirection, targetDirectionVec);

			// ��ǥ Yaw �� ���
			float targetYaw = atan2f(targetDirection.x, targetDirection.z);

			// ���� NPC�� ȸ�� �� �������� (GetLook ���͸� ����Ͽ� Yaw ���)
			XMFLOAT3 currentLook = npc->GetLook();
			float currentYaw = atan2f(currentLook.x, currentLook.z);

			// ��ǥ �������� ȸ��
			npc->Rotate(0.0f, targetYaw - currentYaw, 0.0f);

			// ������ �̵� (���� Look ���� ��������)
			npc->MoveForward(0.4f);

			// ���� ���� Ȯ�� (��: 50 ���� �̳�) �� Attack ���·� ��ȯ�ϴ� ���� �߰� ����
			float attackRange = 50.0f;
			float distanceToPlayer = sqrt(pow(playerPos.x - npcPos.x, 2) + pow(playerPos.y - npcPos.y, 2) + pow(playerPos.z - npcPos.z, 2));
			if (distanceToPlayer < attackRange)
			{
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCAttackState>());
				return;
			}

			// �߰� �� ���� ���� (��: �÷��̾ �ʹ� �ָ� ���)
			float loseRange = 1000.f;
			if (distanceToPlayer > loseRange)
			{
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
				return;
			}
		}
	}
}

void AtkNPCChaseState::Exit(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Spider:
	case GameObjectType::Wasp:
	case GameObjectType::Wolf:
	case GameObjectType::Snake:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(1, false);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}

//=====================================Die=================================================

void AtkNPCDieState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 10 * 1000; // 10�ʰ� �׾�����

	switch (npc->m_objectType)
	{
	case GameObjectType::Wasp:
	case GameObjectType::Snail:
		npc->m_pSkinnedAnimationController->SetTrackEnable(5, true);
		break;
	case GameObjectType::Snake:
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, true);
		break;
	case GameObjectType::Wolf:
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, true);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(7, true);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}

}

void AtkNPCDieState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCRespawnState>());
		return;
	}
}

void AtkNPCDieState::Exit(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Wasp:
	case GameObjectType::Snail:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[5].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(5, false);
		break;
	case GameObjectType::Snake:
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, false);
		break;
	case GameObjectType::Wolf:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[8].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, false);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[7].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(7, false);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}


//=====================================Respawn=================================================

void AtkNPCRespawnState::Enter(std::shared_ptr<CGameObject> npc)
{
	npc->isRender = false;
	starttime = std::chrono::system_clock::now();
	duration_time = 20 * 1000; // 20�ʰ� �Ⱥ��̵���
}

void AtkNPCRespawnState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// ���� ��ġ�� ����
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)npc->terraindata;
		XMFLOAT3 xmf3Scale = pTerrain->GetScale();
		int scale_z = (int)(z / xmf3Scale.z);
		bool bReverseQuad = ((scale_z % 2) != 0);
		float fHeight = pTerrain->GetHeight(x, z, bReverseQuad) + 0.0f;
		float y{};
		if (y < fHeight)y = fHeight;

		npc->SetPosition(x, y, z);
		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
		return;
	}
}

void AtkNPCRespawnState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->isRender = true;
}

//=====================================Attack=================================================


void AtkNPCAttackState::Enter(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Wasp:
	case GameObjectType::Snail:
		npc->m_pSkinnedAnimationController->SetTrackEnable(7, true);
		break;
	case GameObjectType::Snake:
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(11, true);
		break;
	case GameObjectType::Wolf:
		npc->m_pSkinnedAnimationController->SetTrackEnable(10, true);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, true);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
	starttime = std::chrono::system_clock::now();
	duration_time = 1.f * 1000; // 1�ʰ�
}

void AtkNPCAttackState::Execute(std::shared_ptr<CGameObject> npc)
{
	// ���ݸ�� �ð� üũ �� �ٽ� �����ϰ�
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
		return;
	}
	// õõ�� ���°� �ƴ� �ٷ� �÷��̾ �ٶ󺸵��� �ϰ� ����
	if (exec_ms < 0.25 * 1000.f) {
		npc->MoveForward(1.5f);
	}
	auto p_info = npc->m_pScene->GetPlayerInfo();
	if (p_info) {
		if (npc->m_pScene->CollisionCheck(npc.get(), p_info)) {
			if (false == p_info->invincibility) {
				auto obj = dynamic_cast<CMonsterObject*> (npc.get());
				p_info->DecreaseHp(obj->GetAtk());
				p_info->SetInvincibility();
			}
		}
	}
}

void AtkNPCAttackState::Exit(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Wasp:
	case GameObjectType::Snail:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[7].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(7, false);
		break;
	case GameObjectType::Snake:
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[11].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(11, false);
		break;
	case GameObjectType::Wolf:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[10].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(10, false);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, false);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}

//=====================================Hit(�¾��� ���)=================================================

void AtkNPCHitState::Enter(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Wasp:
	case GameObjectType::Snail:
		npc->m_pSkinnedAnimationController->SetTrackEnable(6, true);
		break;
	case GameObjectType::Snake:
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->SetTrackEnable(10, true);
		break;
	case GameObjectType::Wolf:
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, true);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, true);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
	starttime = std::chrono::system_clock::now();
	duration_time = 1.0 * 1000; // 1�ʰ� ����
}

void AtkNPCHitState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
		return;
	}
	if (exec_ms < 0.25 * 1000.f) {
		npc->Rotate(0.f, 180.f, 0.f);
		npc->MoveForward(1.5f);
		npc->Rotate(0.f, 180.f, 0.f);
	}
}

void AtkNPCHitState::Exit(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Wasp:
	case GameObjectType::Snail:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[6].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(6, false);
		break;
	case GameObjectType::Snake:
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[10].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(10, false);
		break;
	case GameObjectType::Wolf:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, false);
		break;
	case GameObjectType::Toad:
		npc->m_pSkinnedAnimationController->m_pAnimationTracks[8].SetPosition(-ANIMATION_CALLBACK_EPSILON);
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, false);
		break;
	default:	// �߸��� Ÿ���̴�.
		break;
	}
}
