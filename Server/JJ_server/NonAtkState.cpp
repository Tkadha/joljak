#include "stdafx.h"
#include "Player.h"
#include "NonAtkState.h"
#include "GameObject.h"
#include "RandomUtil.h"
#include "Terrain.h"
#include "Octree.h"
#include <iostream>



void NonAtkNPCGlobalState::Enter(std::shared_ptr<GameObject> npc)
{
}

void NonAtkNPCGlobalState::Execute(std::shared_ptr<GameObject> npc)
{
	if (is_invincible) {
		auto nowtime = std::chrono::system_clock::now();
		auto exectime = nowtime - starttime;
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
		if (exec_ms > 1.5f * 1000) {
			is_invincible = false;
		}
	}
}

void NonAtkNPCGlobalState::Exit(std::shared_ptr<GameObject> npc)
{
}



//=====================================Standing=================================================


void NonAtkNPCStandingState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
	npc->SetAnimationType(ANIMATION_TYPE::IDLE);

	// 근처에 있는 플레이어에게 타입 보내기
	std::vector<tree_obj*> results;
	XMFLOAT3 oct_distance{ 2500,1000,2500 };
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void NonAtkNPCStandingState::Execute(std::shared_ptr<GameObject> npc)
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

}

void NonAtkNPCStandingState::Exit(std::shared_ptr<GameObject> npc)
{
}


//=====================================Move=================================================



void NonAtkNPCMoveState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
	move_type = rand_type(dre); // 랜덤한 이동 타입(0~2)
	rotate_type = rand_type(dre) % 2; // 랜덤한 회전 타입(0~1)
	npc->SetAnimationType(ANIMATION_TYPE::WALK);

	// 근처에 있는 플레이어에게 타입 보내기
	std::vector<tree_obj*> results;
	XMFLOAT3 oct_distance{ 2500,1000,2500 };
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void NonAtkNPCMoveState::Execute(std::shared_ptr<GameObject> npc)
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
	switch (npc->GetType())
	{
	case OBJECT_TYPE::OB_COW:
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
	case OBJECT_TYPE::OB_PIG:
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
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

	std::vector<tree_obj*> results;
	XMFLOAT3 oct_distance{ 2500,1000,2500 };
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendMovePacket(npc);
		}
	}
}

void NonAtkNPCMoveState::Exit(std::shared_ptr<GameObject> npc)
{
}

//=====================================RunAway=================================================

void NonAtkNPCRunAwayState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	total_time = 0; // 총 시간 초기화
	duration_time = 10 * 1000; // 10초간 도망다님
	move_type = rand_type(dre) % 2; // 랜덤한 이동 타입(0~1)
	rotate_type = rand_type(dre) % 2; // 랜덤한 회전 타입(0~1)
	npc->SetAnimationType(ANIMATION_TYPE::RUN);

	// 근처에 있는 플레이어에게 타입 보내기
	std::vector<tree_obj*> results;
	XMFLOAT3 oct_distance{ 2500,1000,2500 };
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void NonAtkNPCRunAwayState::Execute(std::shared_ptr<GameObject> npc)
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
	switch (npc->GetType())
	{
	case OBJECT_TYPE::OB_COW:
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
	case OBJECT_TYPE::OB_PIG:
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
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());
	std::vector<tree_obj*> results;
	XMFLOAT3 oct_distance{ 2500,1000,2500 };
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendMovePacket(npc);
		}
	}

	if (exec_ms > duration_time / 20)
	{
		move_type = rand_type(dre) % 2;
		rotate_type = rand_type(dre) % 2;
		total_time += exec_ms;
		starttime = std::chrono::system_clock::now();
	}
}

void NonAtkNPCRunAwayState::Exit(std::shared_ptr<GameObject> npc)
{
}

//=====================================Die=================================================

void NonAtkNPCDieState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 10 * 1000; // 10초간 죽어있음

	npc->SetAnimationType(ANIMATION_TYPE::DIE);
	// 근처에 있는 플레이어에게 타입 보내기
	std::vector<tree_obj*> results;
	XMFLOAT3 oct_distance{ 2500,1000,2500 };
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void NonAtkNPCDieState::Execute(std::shared_ptr<GameObject> npc)
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

void NonAtkNPCDieState::Exit(std::shared_ptr<GameObject> npc)
{
}
//=====================================Respawn=================================================

void NonAtkNPCRespawnState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->is_alive = false;
	starttime = std::chrono::system_clock::now();
	duration_time = 20 * 1000; // 20초간 안보이도록
}

void NonAtkNPCRespawnState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		npc->Sethp(20);
		std::pair<float,float> randompos = genRandom::generateRandomXZ(gen, 1000.f, 2000.f, 1000.f, 2000.f);
		XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
		int scale_z = (int)(randompos.second / xmf3Scale.z);
		bool bReverseQuad = ((scale_z % 2) != 0);
		float fHeight = Terrain::terrain->GetHeight(randompos.first, randompos.second, bReverseQuad) + 0.0f;
		float y{};
		if (y < fHeight)y = fHeight;
		
		npc->SetPosition(randompos.first, y, randompos.second);

		Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

		std::vector<tree_obj*> results;
		XMFLOAT3 oct_distance{ 2500,1000,2500 };
		tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
		Octree::PlayerOctree.query(n_obj, oct_distance, results);
		for (auto& p_obj : results) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME)continue;
				if (cl.second->m_id != p_obj->u_id) continue;
				cl.second->SendMovePacket(npc);
			}
		}
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<NonAtkNPCStandingState>());
		return;
	}
}

void NonAtkNPCRespawnState::Exit(std::shared_ptr<GameObject> npc)
{
	npc->is_alive = true;
}
