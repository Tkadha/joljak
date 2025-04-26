#include "AtkState.h"
#include "Object.h"
#include <iostream>
#include "RandomUtil.h"


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
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, true);
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
}

void AtkNPCMoveState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, false);
}

//=====================================Chase=================================================

void AtkNPCChaseState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();

}

void AtkNPCChaseState::Execute(std::shared_ptr<CGameObject> npc)
{
}

void AtkNPCChaseState::Exit(std::shared_ptr<CGameObject> npc)
{
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
}

void AtkNPCAttackState::Execute(std::shared_ptr<CGameObject> npc)
{
}

void AtkNPCAttackState::Exit(std::shared_ptr<CGameObject> npc)
{
}
