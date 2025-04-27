#include "AtkState.h"
#include "Object.h"
#include "Scene.h"
#include <iostream>
#include "RandomUtil.h"
#include <cmath> // sqrt, pow 함수 사용

//=====================================Standing=================================================
void AtkNPCStandingState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
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
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCMoveState>());
		return;
	}

	// 주변에 플레이어가 있는지 확인
	// 플레이어가 있으면 Chase로 변경
	if (npc->m_pScene) {
		auto playerInfo = npc->m_pScene->GetPlayerInfo();
		if (playerInfo)
		{
			XMFLOAT3 playerPos = playerInfo->GetPosition();
			XMFLOAT3 npcPos = npc->GetPosition();

			// 두 위치 사이의 3D 거리 계산
			float distance = sqrt(
				pow(playerPos.x - npcPos.x, 2) +
				pow(playerPos.y - npcPos.y, 2) +
				pow(playerPos.z - npcPos.z, 2)
			);

			// 500 범위 내에 있다면 Chase 상태로 전환
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
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
	move_type = rand_type(dre); // 랜덤한 이동 타입(0~2)
	rotate_type = rand_type(dre) % 2; // 랜덤한 회전 타입(0~1)
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
	defalut:	// 잘못된 타입이다.
		break;
	}
}

void AtkNPCMoveState::Execute(std::shared_ptr<CGameObject> npc)
{
	// 앞으로 이동
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
		return;
	}
	switch (move_type)
	{
	case 0:
		// 전진
		npc->MoveForward(0.2f);
		break;
	case 1:
		// 회전하면서 전진
		npc->Rotate(0.f, 0.5f, 0.f);
		npc->MoveForward(0.1f);
		break;
	case 2:
		// 회전
		npc->Rotate(0.f, 0.25f, 0.f);
		break;
	}

	// standing과 동일하게 위치 기반으로 일정 범위 내면 chase 상태 변경
	if (npc->m_pScene) {
		auto playerInfo = npc->m_pScene->GetPlayerInfo();
		if (playerInfo)
		{
			XMFLOAT3 playerPos = playerInfo->GetPosition();
			XMFLOAT3 npcPos = npc->GetPosition();

			// 두 위치 사이의 3D 거리 계산
			float distance = sqrt(
				pow(playerPos.x - npcPos.x, 2) +
				pow(playerPos.y - npcPos.y, 2) +
				pow(playerPos.z - npcPos.z, 2)
			);

			// 500 범위 내에 있다면 Chase 상태로 전환
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
	defalut:	// 잘못된 타입이다.
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
	defalut:	// 잘못된 타입이다.
		break;
	}
}

void AtkNPCChaseState::Execute(std::shared_ptr<CGameObject> npc)
{
	// 플레이어 위치로 다가감. 일정범위로 붙게되면 attack 상태 변경
	if (npc->m_pScene)
	{
		auto playerInfo = npc->m_pScene->GetPlayerInfo();
		if (playerInfo)
		{
			XMFLOAT3 playerPos = playerInfo->GetPosition();
			XMFLOAT3 npcPos = npc->GetPosition();

			// 플레이어 방향 벡터 계산
			XMVECTOR targetDirectionVec = XMVector3Normalize(XMVectorSet(playerPos.x - npcPos.x, 0.0f, playerPos.z - npcPos.z, 0.0f));
			XMFLOAT3 targetDirection;
			XMStoreFloat3(&targetDirection, targetDirectionVec);

			// 목표 Yaw 값 계산
			float targetYaw = atan2f(targetDirection.x, targetDirection.z);

			// 현재 NPC의 회전 값 가져오기 (GetLook 벡터를 사용하여 Yaw 계산)
			XMFLOAT3 currentLook = npc->GetLook();
			float currentYaw = atan2f(currentLook.x, currentLook.z);

			// 목표 방향으로 회전
			npc->Rotate(0.0f, targetYaw - currentYaw, 0.0f);

			// 앞으로 이동 (현재 Look 벡터 방향으로)
			npc->MoveForward(0.4f);

			// 공격 범위 확인 (예: 50 유닛 이내) 후 Attack 상태로 전환하는 로직 추가 가능
			float attackRange = 50.0f;
			float distanceToPlayer = sqrt(pow(playerPos.x - npcPos.x, 2) + pow(playerPos.y - npcPos.y, 2) + pow(playerPos.z - npcPos.z, 2));
			if (distanceToPlayer < attackRange)
			{
				//npc->FSM_manager->ChangeState(std::make_shared<AtkNPCAttackState>());
				//return;
			}

			// 추격 중 멈춤 조건 (예: 플레이어가 너무 멀리 벗어남)
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
	defalut:	// 잘못된 타입이다.
		break;
	}
}

//=====================================Die=================================================

void AtkNPCDieState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 10 * 1000; // 10초간 죽어있음
	if (npc->m_objectType == GameObjectType::Wolf)
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, true);
	else if (npc->m_objectType == GameObjectType::Toad)
		npc->m_pSkinnedAnimationController->SetTrackEnable(7, true);
	else if (npc->m_objectType == GameObjectType::Wasp)
		npc->m_pSkinnedAnimationController->SetTrackEnable(5, true);
	else
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, true);

}

void AtkNPCDieState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCRespawnState>());
		return;
	}
}

void AtkNPCDieState::Exit(std::shared_ptr<CGameObject> npc)
{
	if (npc->m_objectType == GameObjectType::Wolf)
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, false);
	else if (npc->m_objectType == GameObjectType::Toad)
		npc->m_pSkinnedAnimationController->SetTrackEnable(7, false);
	else if (npc->m_objectType == GameObjectType::Wasp)
		npc->m_pSkinnedAnimationController->SetTrackEnable(5, false);
	else
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, false);
}


//=====================================Respawn=================================================

void AtkNPCRespawnState::Enter(std::shared_ptr<CGameObject> npc)
{
	npc->isRender = false;
	starttime = std::chrono::system_clock::now();
	duration_time = 20 * 1000; // 20초간 안보이도록
}

void AtkNPCRespawnState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 랜덤 위치에 생성
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)npc->terraindata;
		XMFLOAT3 xmf3Scale = pTerrain->GetScale();
		int scale_z = (int)(z / xmf3Scale.z);
		bool bReverseQuad = ((scale_z % 2) != 0);
		float fHeight = pTerrain->GetHeight(x, z, bReverseQuad) + 0.0f;
		float y{};
		if (y < fHeight)y = fHeight;

		npc->SetPosition(x, y, z);
		// 상태 전환
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
	case GameObjectType::Spider:
		break;
	case GameObjectType::Wasp:
		break;
	case GameObjectType::Wolf:
		break;
	case GameObjectType::Snake:
		break;
	case GameObjectType::Bat:
		break;
	case GameObjectType::Toad:
		break;
	case GameObjectType::Turtle:
		break;
	defalut:	// 잘못된 타입이다.
		break;
	}
}

void AtkNPCAttackState::Execute(std::shared_ptr<CGameObject> npc)
{
}

void AtkNPCAttackState::Exit(std::shared_ptr<CGameObject> npc)
{
	switch (npc->m_objectType)
	{
	case GameObjectType::Spider:
		break;
	case GameObjectType::Wasp:
		break;
	case GameObjectType::Wolf:
		break;
	case GameObjectType::Snake:
		break;
	case GameObjectType::Bat:
		break;
	case GameObjectType::Toad:
		break;
	case GameObjectType::Turtle:
		break;
	defalut:	// 잘못된 타입이다.
		break;
	}
}
