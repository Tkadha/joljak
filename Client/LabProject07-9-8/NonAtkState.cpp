#include "NonAtkState.h"
#include "Object.h"
#include <iostream>
#include "RandomUtil.h"


//=====================================Standing=================================================


void NonAtkNPCStandingState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
	std::cout << "Standing State Enter, duration_time: " << duration_time  << std::endl;
	npc->m_pSkinnedAnimationController->SetTrackEnable(0, true);
}

void NonAtkNPCStandingState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if(exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCMoveState>());
		return;
	}
	//auto obj = dynamic_cast<CMonsterObject*>(npc.get());

}

void NonAtkNPCStandingState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->m_pSkinnedAnimationController->SetTrackEnable(0, false);
}


//=====================================Move=================================================



void NonAtkNPCMoveState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
	move_type = rand_type(dre); // 랜덤한 이동 타입(0~2)
	rotate_type = rand_type(dre) % 2; // 랜덤한 회전 타입(0~1)
	std::cout << "Move State Enter, duration_time: " << duration_time << std::endl;
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, true);
}

void NonAtkNPCMoveState::Execute(std::shared_ptr<CGameObject> npc)
{
	// 앞으로 이동
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCStandingState>());
		return;
	}
	switch (npc->m_objectType)
	{
	case GameObjectType::Cow:
	{
		switch (move_type)
		{
		case 0:
			// 전진
			npc->MoveForward(0.2f);
			break;
		case 1:
			// 회전하면서 전진
			if (rotate_type == 0) npc->Rotate(0.f, -0.5f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 0.5f, 0.f);
			npc->MoveForward(0.1f);
			break;
		case 2:
			// 회전
			if (rotate_type == 0) npc->Rotate(0.f, -0.25f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 0.25f, 0.f);
			break;
		}
	}
	break;
	case GameObjectType::Pig:
	{
		switch (move_type)
		{
		case 0:
			// 전진
			npc->MoveForward(0.2f);
			break;
		case 1:
			// 회전하면서 전진
			if (rotate_type == 0) npc->Rotate(0.f, -0.5f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 0.5f, 0.f);
			npc->MoveForward(0.1f);
			break;
		case 2:
			// 회전
			if (rotate_type == 0) npc->Rotate(0.f, -0.25f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 0.25f, 0.f);
			break;
		}
	}
	break;
	default:
		break;
	}
}

void NonAtkNPCMoveState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, false);
}

//=====================================RunAway=================================================

void NonAtkNPCRunAwayState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	total_time = 0; // 총 시간 초기화
	duration_time = 10 * 1000; // 10초간 도망다님
	move_type = rand_type(dre) % 2; // 랜덤한 이동 타입(0~1)
	rotate_type = rand_type(dre) % 2; // 랜덤한 회전 타입(0~1)
	std::cout << "Move State Enter, duration_time: " << duration_time << std::endl;
	if(npc->m_objectType == GameObjectType::Cow)
		npc->m_pSkinnedAnimationController->SetTrackEnable(5, true);
	else if(npc->m_objectType == GameObjectType::Pig)
		npc->m_pSkinnedAnimationController->SetTrackEnable(6, true);

}

void NonAtkNPCRunAwayState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (total_time > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCMoveState>());
		return;
	}
	switch (npc->m_objectType)
	{
	case GameObjectType::Cow:
	{
		switch (move_type)
		{
		case 0:
			// 전진
			npc->MoveForward(0.9f);
			break;
		case 1:
			// 회전하면서 전진
			if (rotate_type == 0) npc->Rotate(0.f, -1.0f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 1.0f, 0.f);
			npc->MoveForward(0.7f);
			break;
		}
	}
	break;
	case GameObjectType::Pig:
	{
		switch (move_type)
		{
		case 0:
			// 전진
			npc->MoveForward(0.9f);
			break;
		case 1:
			// 회전하면서 전진
			if (rotate_type == 0) npc->Rotate(0.f, -1.0f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 1.0f, 0.f);
			npc->MoveForward(0.7f);
			break;
		}
	}
	break;
	default:
		break;
	}
	if (exec_ms > duration_time / 20)
	{
		move_type = rand_type(dre) % 2;
		rotate_type = rand_type(dre) % 2;
		total_time += exec_ms;
		starttime = std::chrono::system_clock::now();
	}
}

void NonAtkNPCRunAwayState::Exit(std::shared_ptr<CGameObject> npc)
{
	if (npc->m_objectType == GameObjectType::Cow)
		npc->m_pSkinnedAnimationController->SetTrackEnable(5, false);
	else if (npc->m_objectType == GameObjectType::Pig)
		npc->m_pSkinnedAnimationController->SetTrackEnable(6, false);
}

//=====================================Die=================================================

void NonAtkNPCDieState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 10 * 1000; // 10초간 죽어있음

	if (npc->m_objectType == GameObjectType::Cow)
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, true);
	else if (npc->m_objectType == GameObjectType::Pig)
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, true);
}

void NonAtkNPCDieState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCRespawnState>());
		return;
	}
}

void NonAtkNPCDieState::Exit(std::shared_ptr<CGameObject> npc)
{
	if (npc->m_objectType == GameObjectType::Cow)
		npc->m_pSkinnedAnimationController->SetTrackEnable(8, false);
	else if (npc->m_objectType == GameObjectType::Pig)
		npc->m_pSkinnedAnimationController->SetTrackEnable(9, false);
}
//=====================================Respawn=================================================

void NonAtkNPCRespawnState::Enter(std::shared_ptr<CGameObject> npc)
{
	npc->isRender = false;
	starttime = std::chrono::system_clock::now();
	duration_time = 20 * 1000; // 20초간 안보이도록
}

void NonAtkNPCRespawnState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
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
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCStandingState>());
		return;
	}
}

void NonAtkNPCRespawnState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->isRender = true;
}
