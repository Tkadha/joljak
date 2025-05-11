#include "AtkState.h"
#include "Object.h"
#include "Scene.h"
#include <iostream>
#include "RandomUtil.h"
#include <cmath> // sqrt, pow 함수 사용
#include <numbers>
#include "Scene.h"

//=====================================Standing==============================================
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

			// 300 범위 내에 있다면 Chase 상태로 전환
			float detectionRange = 300.f;
			if (distance < detectionRange)
			{
				// 플레이어 방향 벡터 계산 (NPC -> 플레이어)
				XMVECTOR toPlayerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&npcPos));
				XMVECTOR normalizedToPlayerVec = XMVector3Normalize(toPlayerVec);

				// NPC의 Look 벡터 가져오기
				XMFLOAT3 npcLook = npc->GetLook();
				XMVECTOR npcLookVec = XMLoadFloat3(&npcLook);

				// 두 벡터 사이의 각도 계산 (내적 사용)
				XMVECTOR dotProductVec = XMVector3Dot(normalizedToPlayerVec, npcLookVec);
				float dotProduct;
				XMStoreFloat(&dotProduct, dotProductVec);

				// acosf의 입력값은 [-1, 1] 범위 내에 있어야 안전합니다.
				float angleBetween = acosf(std::max(-1.0f, std::min(1.0f, dotProduct)));

				// 시야각 (30도를 라디안으로 변환)
				float fieldOfView = XMConvertToRadians(60.0f);

				// 시야각 내에 있다면 Chase 상태로 전환
				if (angleBetween <= fieldOfView * 0.5f)
				{
					npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
					return;
				}
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
	default:	// 잘못된 타입이다.
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
	//npc->m_pScene->octree.update(npc->m_treecount, npc->GetPosition());

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
			// 300 범위 내에 있다면 Chase 상태로 전환
			float detectionRange = 300.f;
			if (distance < detectionRange)
			{
				// 플레이어 방향 벡터 계산 (NPC -> 플레이어)
				XMVECTOR toPlayerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&npcPos));
				XMVECTOR normalizedToPlayerVec = XMVector3Normalize(toPlayerVec);

				// NPC의 Look 벡터 가져오기
				XMFLOAT3 npcLook = npc->GetLook();
				XMVECTOR npcLookVec = XMLoadFloat3(&npcLook);

				// 두 벡터 사이의 각도 계산 (내적 사용)
				XMVECTOR dotProductVec = XMVector3Dot(normalizedToPlayerVec, npcLookVec);
				float dotProduct;
				XMStoreFloat(&dotProduct, dotProductVec);

				// acosf의 입력값은 [-1, 1] 범위 내에 있어야 안전합니다.
				float angleBetween = acosf(std::max(-1.0f, std::min(1.0f, dotProduct)));

				// 시야각 (30도를 라디안으로 변환)
				float fieldOfView = XMConvertToRadians(60.0f);

				// 시야각 내에 있다면 Chase 상태로 전환
				if (angleBetween <= fieldOfView * 0.5f)
				{
					npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
					return;
				}
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
	default:	// 잘못된 타입이다.
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
	default:	// 잘못된 타입이다.
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


			// NPC의 Look 벡터 가져오기
			XMFLOAT3 npcLook = npc->GetLook();
			XMVECTOR npcLookVec = XMLoadFloat3(&npcLook);
			XMFLOAT3 npcLookNorm;
			XMStoreFloat3(&npcLookNorm, XMVector3Normalize(npcLookVec)); // Look 벡터도 정규화

			// 수평면에서의 NPC Look 벡터 (Y 성분 0으로 설정 후 정규화)
			XMVECTOR npcLookVecXZ = XMVector3Normalize(XMVectorSet(npcLookNorm.x, 0.0f, npcLookNorm.z, 0.0f));

			// 목표 Yaw 값 계산 (수평 방향 벡터 사용)
			float targetYaw = atan2f(targetDirection.x, targetDirection.z);

			// 현재 NPC의 Yaw 값 계산 (수평 Look 벡터 사용)
			float currentYaw = atan2f(npcLookNorm.x, npcLookNorm.z);

			float deltaYaw = targetYaw - currentYaw;

			if (deltaYaw > std::numbers::pi_v<float>)
			{
				deltaYaw -= 2 * std::numbers::pi_v<float>;
			}
			else if (deltaYaw < -std::numbers::pi_v<float>)
			{
				deltaYaw += 2 * std::numbers::pi_v<float>;
			}
			// 목표 방향으로 회전
			npc->Rotate(0.0f, deltaYaw*2, 0.0f);


			// 공격 범위 확인 (예: 30 유닛 이내) 후 Attack 상태로 전환하는 로직 추가 가능
			float attackRange = 30.0f;
			float distanceToPlayer = sqrt(pow(playerPos.x - npcPos.x, 2) + pow(playerPos.y - npcPos.y, 2) + pow(playerPos.z - npcPos.z, 2));
			
			if (distanceToPlayer < attackRange)
			{
				auto obj = dynamic_cast<CMonsterObject*> (npc.get());
				if(obj->FSM_manager->GetAtkDelay() == false)
					npc->FSM_manager->ChangeState(std::make_shared<AtkNPCAttackState>());
				return;
			}

			// 앞으로 이동 (현재 Look 벡터 방향으로)
			npc->MoveForward(0.4f);

			//npc->m_pScene->octree.update(npc->m_treecount, npc->GetPosition());

			// 추격 중 멈춤 조건 (예: 플레이어가 너무 멀리 벗어남)
			float loseRange = 700.f;
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
	default:	// 잘못된 타입이다.
		break;
	}
}

//=====================================Die=================================================

void AtkNPCDieState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 10 * 1000; // 10초간 죽어있음

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
	default:	// 잘못된 타입이다.
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
		// 상태 전환
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
	default:	// 잘못된 타입이다.
		break;
	}
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
		auto obj = dynamic_cast<CMonsterObject*> (npc.get());
		obj->Sethp(20);

		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)npc->terraindata;
		XMFLOAT3 xmf3Scale = pTerrain->GetScale();
		int scale_z = (int)(z / xmf3Scale.z);
		bool bReverseQuad = ((scale_z % 2) != 0);
		float fHeight = pTerrain->GetHeight(x, z, bReverseQuad) + 0.0f;
		float y{};
		if (y < fHeight)y = fHeight;

		npc->SetPosition(x, y, z);
		//npc->m_pScene->octree.update(npc->m_treecount, npc->GetPosition());

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
	default:	// 잘못된 타입이다.
		break;
	}
	starttime = std::chrono::system_clock::now();
	duration_time = 1.f * 1000; // 1초간
}

void AtkNPCAttackState::Execute(std::shared_ptr<CGameObject> npc)
{
	// 공격모션 시간 체크 후 다시 추적하게
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
		return;
	}
	if (exec_ms < 0.25 * 1000.f) {
		npc->MoveForward(1.5f);
	}
	//npc->m_pScene->octree.update(npc->m_treecount, npc->GetPosition());

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
	default:	// 잘못된 타입이다.
		break;
	}
	auto obj = dynamic_cast<CMonsterObject*> (npc.get());
	npc->FSM_manager->SetAtkDelay();
}

//=====================================Hit(맞았을 경우)=================================================

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
	default:	// 잘못된 타입이다.
		break;
	}
	starttime = std::chrono::system_clock::now();
	duration_time = 1.0 * 1000; // 1초간 진행
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
	//npc->m_pScene->octree.update(npc->m_treecount, npc->GetPosition());

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
	default:	// 잘못된 타입이다.
		break;
	}
}



void AtkNPCGlobalState::Enter(std::shared_ptr<CGameObject> npc)
{
}

void AtkNPCGlobalState::Execute(std::shared_ptr<CGameObject> npc)
{
	if (is_invincible) {
		auto nowtime = std::chrono::system_clock::now();
		auto exectime = nowtime - starttime;
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
		if (exec_ms > 1.5f * 1000) {
			is_invincible = false;
		}
	}
	if (is_atkdelay) {
		auto nowtime = std::chrono::system_clock::now();
		auto exectime = nowtime - atk_delay_starttime;
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
		if (exec_ms > 1.f * 1000) {
			is_atkdelay = false;
		}
	}
}

void AtkNPCGlobalState::Exit(std::shared_ptr<CGameObject> npc)
{
}
