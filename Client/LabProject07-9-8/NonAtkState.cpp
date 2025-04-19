#include "NonAtkState.h"
#include "Object.h"
#include <iostream>
#include "RandomUtil.h"




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
	switch (move_type)
	{
	case 0:
		// ����
		npc->MoveForward(0.2f);
		break;
	case 1:
		// ȸ���ϸ鼭 ����
		if(rotate_type == 0) npc->Rotate(0.f, -0.5f, 0.f);
		else if(rotate_type == 1) npc->Rotate(0.f, 0.5f, 0.f);
		npc->MoveForward(0.1f);
		break;
	case 2:
		// ȸ��
		if (rotate_type == 0) npc->Rotate(0.f, -0.25f, 0.f);
		else if (rotate_type == 1) npc->Rotate(0.f, 0.25f, 0.f);
		break;
	}

}

void NonAtkNPCMoveState::Exit(std::shared_ptr<CGameObject> npc)
{
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, false);
}


void NonAtkNPCRunAwayState::Enter(std::shared_ptr<CGameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	total_time = 0; // �� �ð� �ʱ�ȭ
	duration_time = 10 * 1000; // 10�ʰ� �����ٴ�
	move_type = rand_type(dre) % 2; // ������ �̵� Ÿ��(0~1)
	rotate_type = rand_type(dre) % 2; // ������ ȸ�� Ÿ��(0~1)
	std::cout << "Move State Enter, duration_time: " << duration_time << std::endl;
	npc->m_pSkinnedAnimationController->SetTrackEnable(2, true);
}

void NonAtkNPCRunAwayState::Execute(std::shared_ptr<CGameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (total_time > duration_time)
	{
		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCStandingState>());
		return;
	}
	switch (move_type)
	{
	case 0:
		// ����
		npc->MoveForward(0.4f);
		break;
	case 1:
		// ȸ���ϸ鼭 ����
		if (rotate_type == 0) npc->Rotate(0.f, -0.5f, 0.f);
		else if (rotate_type == 1) npc->Rotate(0.f, 0.5f, 0.f);
		npc->MoveForward(0.2f);
		break;
	}
	if (exec_ms > duration_time / 4)
	{
		move_type = rand_type(dre) % 2;
		rotate_type = rand_type(dre) % 2;
		total_time += exec_ms;
		starttime = std::chrono::system_clock::now();
	}
}

void NonAtkNPCRunAwayState::Exit(std::shared_ptr<CGameObject> npc)
{
}
