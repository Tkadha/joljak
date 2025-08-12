#include "stdafx.h"
#include "Player.h"
#include "BossState.h"
#include "GameObject.h"
#include "RandomUtil.h"
#include "Terrain.h"
#include "Octree.h"

#include <iostream>
#include <cmath> // sqrt, pow 함수 사용
#include <random>

constexpr float pi_f = 3.1415927f;

//=====================================Standing==============================================
void BossStandingState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1500; // 랜덤한 시간(1~3초)을 밀리초로 변환
	npc->SetAnimationType(ANIMATION_TYPE::IDLE);

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void BossStandingState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<BossMoveState>());
		return;
	}

	//주변에 플레이어가 있는지 확인
	//플레이어가 있으면 Chase로 변경
	g_clients_mutex.lock();
	for (auto& pl : PlayerClient::PlayerClients) {
		if (pl.second->state != PC_INGAME) continue;
		auto playerInfo = pl.second;
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
			float detectionRange = 200.f;
			if (distance < detectionRange)
			{
				g_clients_mutex.unlock();
				npc->FSM_manager->ChangeState(std::make_shared<BossChaseState>());
				return;
			}
		}
	}
	g_clients_mutex.unlock();

}

void BossStandingState::Exit(std::shared_ptr<GameObject> npc)
{
}
//=====================================Move=================================================

void BossMoveState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // 랜덤한 시간(1~3초)을 밀리초로 변환
	move_type = rand_type(dre); // 랜덤한 이동 타입(0~2)
	rotate_type = rand_type(dre) % 2; // 랜덤한 회전 타입(0~1)
	npc->SetAnimationType(ANIMATION_TYPE::WALK);

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}

}

void BossMoveState::Execute(std::shared_ptr<GameObject> npc)
{
	// 앞으로 이동
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<BossStandingState>());
		return;
	}
	switch (move_type)
	{
	case 0:
		// 전진
		npc->MoveForward(0.15f);
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
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;

			cl.second->SendMovePacket(npc);
		}
	}

	//주변에 플레이어가 있는지 확인
	//플레이어가 있으면 Chase로 변경
	g_clients_mutex.lock();
	for (auto& pl : PlayerClient::PlayerClients) {
		if (pl.second->state != PC_INGAME) continue;
		auto playerInfo = pl.second;
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
			float detectionRange = 200.f;
			if (distance < detectionRange)
			{
				g_clients_mutex.unlock();
				npc->FSM_manager->ChangeState(std::make_shared<BossChaseState>());
				return;
			}
		}
	}
	g_clients_mutex.unlock();
}

void BossMoveState::Exit(std::shared_ptr<GameObject> npc)
{

}

//=====================================Chase=================================================

void BossChaseState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->SetAnimationType(ANIMATION_TYPE::WALK);
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
	float near_player_distance{ 1000.f };
	for (auto& t_obj : results) {
		XMFLOAT3 playerPos = t_obj->position;
		XMFLOAT3 npcPos = npc->GetPosition();
		float distance = sqrt(
			pow(playerPos.x - npcPos.x, 2) +
			pow(playerPos.y - npcPos.y, 2) +
			pow(playerPos.z - npcPos.z, 2)
		);
		if (distance < near_player_distance) {
			near_player_distance = distance;
			aggro_player_id = t_obj->u_id;
		}
	}
}

void BossChaseState::Execute(std::shared_ptr<GameObject> npc)
{
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,1000,500 }, results);
	if (results.size() <= 0)
	{
		npc->FSM_manager->ChangeState(std::make_shared<BossStandingState>());
		return;
	}


	g_clients_mutex.lock();
	for (auto& cl : PlayerClient::PlayerClients) {
		if (cl.second->m_id != aggro_player_id) continue;
		auto& player = cl.second;
		XMFLOAT3 playerPos = player->GetPosition();
		XMFLOAT3 npcPos = npc->GetPosition();
		// 플레이어 방향 벡터 계산
		XMVECTOR targetDirectionVec = XMVector3Normalize(XMVectorSet(playerPos.x - npcPos.x, 0.0f, playerPos.z - npcPos.z, 0.0f));
		XMFLOAT3 targetDirection;
		XMStoreFloat3(&targetDirection, targetDirectionVec);


		// NPC의 Look 벡터 가져오기
		XMVECTOR npcLookVec = XMLoadFloat3(&npc->GetLook());
		XMFLOAT3 npcLookNorm;
		XMStoreFloat3(&npcLookNorm, XMVector3Normalize(npcLookVec)); // Look 벡터도 정규화
		XMVECTOR npcLookVecXZ = XMVector3Normalize(XMVectorSet(npcLookNorm.x, 0.0f, npcLookNorm.z, 0.0f));

		float targetYaw = atan2f(targetDirection.x, targetDirection.z);
		float currentYaw = atan2f(npcLookNorm.x, npcLookNorm.z);
		float deltaYaw = targetYaw - currentYaw;

		if (deltaYaw > pi_f) deltaYaw -= 2 * pi_f;
		else if (deltaYaw < -pi_f) deltaYaw += 2 * pi_f;

		// 목표 방향으로 회전
		npc->Rotate(0.0f, deltaYaw * 2.f, 0.0f);

		float attackRange = 100.0f;
		if (BossAttackState::Sp_atk_counter > 5) {
			attackRange = 250.f;
		}
		float distanceToPlayer = sqrt(pow(playerPos.x - npcPos.x, 2) + pow(playerPos.y - npcPos.y, 2) + pow(playerPos.z - npcPos.z, 2));

		if (distanceToPlayer < attackRange)
		{
			const float BOSS_FOV_DEGREES = 90.0f;

			// 보스에서 플레이어로 향하는 방향 벡터 (정규화)
			XMVECTOR toPlayerVec = XMVector3Normalize(XMVectorSet(playerPos.x - npcPos.x, playerPos.y - npcPos.y, playerPos.z - npcPos.z, 0.0f));
			// 보스의 정면 방향 벡터 (정규화)
			XMVECTOR normalizedNpcLookVec = XMVector3Normalize(npcLookVec);

			// 두 벡터의 내적(dot product) 계산
			float dot = XMVectorGetX(XMVector3Dot(normalizedNpcLookVec, toPlayerVec));

			// 시야각의 절반(half-angle)의 코사인 값 계산
			float cosHalfFov = cosf(XMConvertToRadians(BOSS_FOV_DEGREES / 2.0f));

			std::vector<tree_obj*> results;
			tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
			Octree::PlayerOctree.query(n_obj, oct_distance, results);
			for (auto& p_obj : results) {
				for (auto& cl : PlayerClient::PlayerClients) {
					if (cl.second->state != PC_INGAME)continue;
					if (cl.second->m_id != p_obj->u_id) continue;

					cl.second->SendMovePacket(npc);
				}
			}

			if (dot >= cosHalfFov)
			{
				g_clients_mutex.unlock();
				if (npc->FSM_manager->GetAtkDelay() == false) {
					npc->FSM_manager->ChangeState(std::make_shared<BossAttackState>());
				}
				// 공격 상태로 전환하더라도 아래 로직은 실행하지 않으므로 여기서 return
				return;
			}
		}

		npc->MoveForward(0.1f);

		Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());
		std::vector<tree_obj*> results;
		tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
		Octree::PlayerOctree.query(n_obj, oct_distance, results);
		for (auto& p_obj : results) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME)continue;
				if (cl.second->m_id != p_obj->u_id) continue;

				cl.second->SendMovePacket(npc);
			}
		}
		// 추격 중 멈춤 조건 (예: 플레이어가 너무 멀리 벗어남)
		float loseRange = 400.f;
		if (distanceToPlayer > loseRange)
		{
			g_clients_mutex.unlock();
			npc->FSM_manager->ChangeState(std::make_shared<BossStandingState>());
			return;
		}
		break;
	}
	g_clients_mutex.unlock();
}

void BossChaseState::Exit(std::shared_ptr<GameObject> npc)
{

}
//=====================================Die=================================================

void BossDieState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 25 * 1000; // 25초간 죽어있음

	npc->SetAnimationType(ANIMATION_TYPE::DIE);
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}

}

void BossDieState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<BossRespawnState>());
		return;
	}
}

void BossDieState::Exit(std::shared_ptr<GameObject> npc)
{
}


//=====================================Respawn=================================================

void BossRespawnState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->is_alive = false;
	starttime = std::chrono::system_clock::now();
	duration_time = 300 * 1000; // 5분간 안보이도록


	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);

	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendRemovePacket(npc);
		}
	}
}

void BossRespawnState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// 랜덤 위치에 생성
		npc->Sethp(20);

		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, 1000.f, 2000.f, 1000.f, 2000.f);
		XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
		int scale_z = (int)(randompos.second / xmf3Scale.z);
		bool bReverseQuad = ((scale_z % 2) != 0);
		float fHeight = Terrain::terrain->GetHeight(randompos.first, randompos.second, bReverseQuad) + 0.0f;
		float y{};
		if (y < fHeight) y = fHeight;
		if (npc->GetType() == OBJECT_TYPE::OB_BAT)
			npc->fly_height = 13.f;
		npc->SetPosition(randompos.first, y, randompos.second);

		Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

		// 상태 전환
		npc->FSM_manager->ChangeState(std::make_shared<BossStandingState>());
		return;
	}
}

void BossRespawnState::Exit(std::shared_ptr<GameObject> npc)
{
	npc->is_alive = true;

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);

}

//=====================================Attack=================================================

int BossAttackState::Sp_atk_counter = 0;
void BossAttackState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 2.65f * 1000; // 1초간
	Sp_atk_counter++;
	if (Sp_atk_counter > 5) {
		Sp_atk_counter = 0;
		npc->SetAnimationType(ANIMATION_TYPE::SPECIAL_ATTACK);
		duration_time = 2.f * 1000; // 스페셜 공격은 2초간
	}
	else {
		npc->SetAnimationType(ANIMATION_TYPE::ATTACK);
	}

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void BossAttackState::Execute(std::shared_ptr<GameObject> npc)
{
	// 공격모션 시간 체크 후 다시 추적하게
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<BossChaseState>());
		return;
	}
	if (exec_ms < 0.25 * 1000.f) {
		auto n_type = npc->GetAnimationType();
		if(n_type == ANIMATION_TYPE::SPECIAL_ATTACK)
			npc->MoveForward(0.1f);
		else
			npc->MoveForward(0.5f);
	}
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendMovePacket(npc);
		}
	}
}

void BossAttackState::Exit(std::shared_ptr<GameObject> npc)
{
	npc->FSM_manager->SetAtkDelay();
}

//=====================================Hit(맞았을 경우)=================================================

void BossHitState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->SetAnimationType(ANIMATION_TYPE::HIT);
	starttime = std::chrono::system_clock::now();
	duration_time = 1.5f * 1000; // 1초간 진행
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void BossHitState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<BossChaseState>());
		return;
	}
	if (exec_ms < 0.25 * 1000.f) {
		npc->MoveForward(-0.75f);
	}
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		std::lock_guard<std::mutex> lock(g_clients_mutex);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendMovePacket(npc);
		}
	}
}

void BossHitState::Exit(std::shared_ptr<GameObject> npc)
{
}



void BossGlobalState::Enter(std::shared_ptr<GameObject> npc)
{
}

void BossGlobalState::Execute(std::shared_ptr<GameObject> npc)
{
	if (is_invincible) {
		auto nowtime = std::chrono::system_clock::now();
		auto exectime = nowtime - starttime;
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
		if (exec_ms > 1.5f * 1000) {
			is_invincible = false;
			std::vector<tree_obj*> results;
			tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
			Octree::PlayerOctree.query(n_obj, oct_distance, results);
			for (auto& p_obj : results) {
				std::lock_guard<std::mutex> lock(g_clients_mutex);
				for (auto& cl : PlayerClient::PlayerClients) {
					if (cl.second->state != PC_INGAME) continue;
					if (cl.second->m_id != p_obj->u_id) continue;
					cl.second->SendInvinciblePacket(npc->GetID(), is_invincible);
				}
			}
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

void BossGlobalState::Exit(std::shared_ptr<GameObject> npc)
{
}