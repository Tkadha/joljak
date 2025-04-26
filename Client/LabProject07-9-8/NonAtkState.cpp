#include "NonAtkState.h"
#include "Object.h"
#include <iostream>
#include "RandomUtil.h"


//=====================================Standing=================================================


void NonAtkNPCStandingState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // ������ �ð�(1~3��)�� �и��ʷ� ��ȯ
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
		// ���� ��ȯ
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
	duration_time = rand_time(dre) * 1000; // ������ �ð�(1~3��)�� �и��ʷ� ��ȯ
	move_type = rand_type(dre); // ������ �̵� Ÿ��(0~2)
	rotate_type = rand_type(dre) % 2; // ������ ȸ�� Ÿ��(0~1)
	std::cout << "Move State Enter, duration_time: " << duration_time << std::endl;
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, true);
}

void NonAtkNPCMoveState::Execute(std::shared_ptr<CGameObject> npc)
{
	// ������ �̵�
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// ���� ��ȯ
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
			// ����
			npc->MoveForward(0.2f);
			break;
		case 1:
			// ȸ���ϸ鼭 ����
			if (rotate_type == 0) npc->Rotate(0.f, -0.5f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 0.5f, 0.f);
			npc->MoveForward(0.1f);
			break;
		case 2:
			// ȸ��
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
			// ����
			npc->MoveForward(0.2f);
			break;
		case 1:
			// ȸ���ϸ鼭 ����
			if (rotate_type == 0) npc->Rotate(0.f, -0.5f, 0.f);
			else if (rotate_type == 1) npc->Rotate(0.f, 0.5f, 0.f);
			npc->MoveForward(0.1f);
			break;
		case 2:
			// ȸ��
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
	total_time = 0; // �� �ð� �ʱ�ȭ
	duration_time = 10 * 1000; // 10�ʰ� �����ٴ�
	move_type = rand_type(dre) % 2; // ������ �̵� Ÿ��(0~1)
	rotate_type = rand_type(dre) % 2; // ������ ȸ�� Ÿ��(0~1)
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
		// ���� ��ȯ
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
			// ����
			npc->MoveForward(0.9f);
			break;
		case 1:
			// ȸ���ϸ鼭 ����
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
			// ����
			npc->MoveForward(0.9f);
			break;
		case 1:
			// ȸ���ϸ鼭 ����
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
	duration_time = 10 * 1000; // 10�ʰ� �׾�����

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
		// ���� ��ȯ
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
	duration_time = 20 * 1000; // 20�ʰ� �Ⱥ��̵���
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
		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCStandingState>());
		return;
	}
}

void NonAtkNPCRespawnState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->isRender = true;
}
