#include "stdafx.h"
#include "Player.h"
#include "AtkState.h"
#include "GameObject.h"
#include "RandomUtil.h"
#include "Terrain.h"
#include "Octree.h"
#include <iostream>
#include <cmath> // sqrt, pow �Լ� ���
#include <random>

constexpr float pi_f = 3.1415927f;

//=====================================Standing==============================================
void AtkNPCStandingState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // ������ �ð�(1~3��)�� �и��ʷ� ��ȯ
	npc->SetAnimationType(ANIMATION_TYPE::IDLE);

	std::vector<tree_obj*> results;
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

void AtkNPCStandingState::Execute(std::shared_ptr<GameObject> npc)
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

	//�ֺ��� �÷��̾ �ִ��� Ȯ��
	//�÷��̾ ������ Chase�� ����
	for (auto& pl : PlayerClient::PlayerClients) {
		if (pl.second->state != PC_INGAME) continue;
		auto playerInfo = pl.second;
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

			// 300 ���� ���� �ִٸ� Chase ���·� ��ȯ
			float detectionRange = 200.f;
			if (distance < detectionRange)
			{
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
				return;
			}
		}
	}
}

void AtkNPCStandingState::Exit(std::shared_ptr<GameObject> npc)
{
}




//=====================================Move=================================================

void AtkNPCMoveState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = rand_time(dre) * 1000; // ������ �ð�(1~3��)�� �и��ʷ� ��ȯ
	move_type = rand_type(dre); // ������ �̵� Ÿ��(0~2)
	rotate_type = rand_type(dre) % 2; // ������ ȸ�� Ÿ��(0~1)
	npc->SetAnimationType(ANIMATION_TYPE::WALK);

	std::vector<tree_obj*> results;
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

void AtkNPCMoveState::Execute(std::shared_ptr<GameObject> npc)
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
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			if(cl.second->viewlist.find(npc->GetID()) == cl.second->viewlist.end())
				cl.second->SendAddPacket(npc);
			cl.second->SendMovePacket(npc);
		}
	}

	//�ֺ��� �÷��̾ �ִ��� Ȯ��
	//�÷��̾ ������ Chase�� ����
	for (auto& pl : PlayerClient::PlayerClients) {
		if (pl.second->state != PC_INGAME) continue;
		auto playerInfo = pl.second;
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

			// 300 ���� ���� �ִٸ� Chase ���·� ��ȯ
			float detectionRange = 200.f;
			if (distance < detectionRange)
			{
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
				return;
			}
		}
	}
}

void AtkNPCMoveState::Exit(std::shared_ptr<GameObject> npc)
{
	
}

//=====================================Chase=================================================

void AtkNPCChaseState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->SetAnimationType(ANIMATION_TYPE::WALK);
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
	std::cout << "enter chasestate\n";
}

void AtkNPCChaseState::Execute(std::shared_ptr<GameObject> npc)
{
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,1000,500 }, results);
	if (results.size() <= 0)
	{
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
		return;
	}
	long long near_player_id{ 0 };
	float near_player_distance{1000.f};
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
			near_player_id = t_obj->u_id;
		}
	}


	for (auto& cl : PlayerClient::PlayerClients) {
		if (cl.second->m_id != near_player_id) continue;

		XMFLOAT3 playerPos = cl.second->GetPosition();
		XMFLOAT3 npcPos = npc->GetPosition();
		// �÷��̾� ���� ���� ���
		XMVECTOR targetDirectionVec = XMVector3Normalize(XMVectorSet(playerPos.x - npcPos.x, 0.0f, playerPos.z - npcPos.z, 0.0f));
		XMFLOAT3 targetDirection;
		XMStoreFloat3(&targetDirection, targetDirectionVec);


		// NPC�� Look ���� ��������
		XMFLOAT3 npcLook = npc->GetLook();
		XMVECTOR npcLookVec = XMLoadFloat3(&npcLook);
		XMFLOAT3 npcLookNorm;
		XMStoreFloat3(&npcLookNorm, XMVector3Normalize(npcLookVec)); // Look ���͵� ����ȭ

		// ����鿡���� NPC Look ���� (Y ���� 0���� ���� �� ����ȭ)
		XMVECTOR npcLookVecXZ = XMVector3Normalize(XMVectorSet(npcLookNorm.x, 0.0f, npcLookNorm.z, 0.0f));

		// ��ǥ Yaw �� ��� (���� ���� ���� ���)
		float targetYaw = atan2f(targetDirection.x, targetDirection.z);

		// ���� NPC�� Yaw �� ��� (���� Look ���� ���)
		float currentYaw = atan2f(npcLookNorm.x, npcLookNorm.z);

		float deltaYaw = targetYaw - currentYaw;

		if (deltaYaw > pi_f)
		{
			deltaYaw -= 2 * pi_f;
		}
		else if (deltaYaw < -pi_f)
		{
			deltaYaw += 2 * pi_f;
		}
		// ��ǥ �������� ȸ��
		npc->Rotate(0.0f, deltaYaw * 2, 0.0f);



		float attackRange = 30.0f;
		float distanceToPlayer = sqrt(pow(playerPos.x - npcPos.x, 2) + pow(playerPos.y - npcPos.y, 2) + pow(playerPos.z - npcPos.z, 2));

		if (distanceToPlayer < attackRange)
		{
			std::vector<tree_obj*> results;
			tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
			Octree::PlayerOctree.query(n_obj, oct_distance, results);
			for (auto& p_obj : results) {
				for (auto& cl : PlayerClient::PlayerClients) {
					if (cl.second->state != PC_INGAME)continue;
					if (cl.second->m_id != p_obj->u_id) continue;
					if (cl.second->viewlist.find(npc->GetID()) == cl.second->viewlist.end())
						cl.second->SendAddPacket(npc);
					cl.second->SendMovePacket(npc);
				}
			}
			auto obj = dynamic_cast<GameObject*> (npc.get());
			if (obj->FSM_manager->GetAtkDelay() == false)
				npc->FSM_manager->ChangeState(std::make_shared<AtkNPCAttackState>());
			return;
		}
		if (distanceToPlayer > attackRange)
			npc->MoveForward(0.2f);
		Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());
		std::vector<tree_obj*> results;
		tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
		Octree::PlayerOctree.query(n_obj, oct_distance, results);
		for (auto& p_obj : results) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME)continue;
				if (cl.second->m_id != p_obj->u_id) continue;
				if (cl.second->viewlist.find(npc->GetID()) == cl.second->viewlist.end())
					cl.second->SendAddPacket(npc);
				cl.second->SendMovePacket(npc);
			}
		}
		// �߰� �� ���� ���� (��: �÷��̾ �ʹ� �ָ� ���)
		float loseRange = 600.f;
		if (distanceToPlayer > loseRange)
		{
			npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
			return;
		}
		break;
	}
}

void AtkNPCChaseState::Exit(std::shared_ptr<GameObject> npc)
{

}

//=====================================Die=================================================

void AtkNPCDieState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 10 * 1000; // 10�ʰ� �׾�����

	npc->SetAnimationType(ANIMATION_TYPE::DIE);
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}

}

void AtkNPCDieState::Execute(std::shared_ptr<GameObject> npc)
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

void AtkNPCDieState::Exit(std::shared_ptr<GameObject> npc)
{
}


//=====================================Respawn=================================================

void AtkNPCRespawnState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->is_alive = false;
	starttime = std::chrono::system_clock::now();
	duration_time = 20 * 1000; // 20�ʰ� �Ⱥ��̵���


	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);

	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendRemovePacket(npc);
		}
	}
}

void AtkNPCRespawnState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time)
	{
		// ���� ��ġ�� ����
		npc->Sethp(20);

		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, 1000.f, 2000.f, 1000.f, 2000.f);
		XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
		int scale_z = (int)(randompos.second / xmf3Scale.z);
		bool bReverseQuad = ((scale_z % 2) != 0);
		float fHeight = Terrain::terrain->GetHeight(randompos.first, randompos.second, bReverseQuad) + 0.0f;
		float y{};
		if (y < fHeight) y = fHeight;

		npc->SetPosition(randompos.first, y, randompos.second);

		Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

		// ���� ��ȯ
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCStandingState>());
		return;
	}
}

void AtkNPCRespawnState::Exit(std::shared_ptr<GameObject> npc)
{
	npc->is_alive = true;

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);

	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAddPacket(npc);
		}
	}
}

//=====================================Attack=================================================


void AtkNPCAttackState::Enter(std::shared_ptr<GameObject> npc)
{
	starttime = std::chrono::system_clock::now();
	duration_time = 1.f * 1000; // 1�ʰ�
	npc->SetAnimationType(ANIMATION_TYPE::ATTACK);
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void AtkNPCAttackState::Execute(std::shared_ptr<GameObject> npc)
{
	// ���ݸ�� �ð� üũ �� �ٽ� �����ϰ�
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
		return;
	}
	if (exec_ms < 0.25 * 1000.f) {
		npc->MoveForward(0.75f);
	}
	Octree::GameObjectOctree.update(npc->GetID(), npc->GetPosition());

	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			if (cl.second->viewlist.find(npc->GetID()) == cl.second->viewlist.end())
				cl.second->SendAddPacket(npc);
			cl.second->SendMovePacket(npc);
		}
	}
	// �÷��̾� ü�� ���� ��Ű��
	// �� ���۵� �����ؾ���

	// �浹ó�� ��� �Ұ��ΰ�
	/*auto p_info = npc->m_pScene->GetPlayerInfo();
	if (p_info) {
		if (npc->m_pScene->CollisionCheck(npc.get(), p_info)) {
			if (false == p_info->invincibility) {
				auto obj = dynamic_cast<CMonsterObject*> (npc.get());
				p_info->DecreaseHp(obj->GetAtk());
				p_info->SetInvincibility();
			}
		}
	}*/
}

void AtkNPCAttackState::Exit(std::shared_ptr<GameObject> npc)
{
	npc->FSM_manager->SetAtkDelay();
}

//=====================================Hit(�¾��� ���)=================================================

void AtkNPCHitState::Enter(std::shared_ptr<GameObject> npc)
{
	npc->SetAnimationType(ANIMATION_TYPE::HIT);
	starttime = std::chrono::system_clock::now();
	duration_time = 1.0f * 1000; // 1�ʰ� ����
	std::vector<tree_obj*> results;
	tree_obj n_obj{ npc->GetID(),npc->GetPosition() };
	Octree::PlayerOctree.query(n_obj, oct_distance, results);
	for (auto& p_obj : results) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->m_id != p_obj->u_id) continue;
			cl.second->SendAnimationPacket(npc);
		}
	}
}

void AtkNPCHitState::Execute(std::shared_ptr<GameObject> npc)
{
	endtime = std::chrono::system_clock::now();
	auto exectime = endtime - starttime;
	auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
	if (exec_ms > duration_time) {
		npc->FSM_manager->ChangeState(std::make_shared<AtkNPCChaseState>());
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
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME)continue;
			if (cl.second->m_id != p_obj->u_id) continue;
			if (cl.second->viewlist.find(npc->GetID()) == cl.second->viewlist.end())
				cl.second->SendAddPacket(npc);
			cl.second->SendMovePacket(npc);
		}
	}
}

void AtkNPCHitState::Exit(std::shared_ptr<GameObject> npc)
{
}



void AtkNPCGlobalState::Enter(std::shared_ptr<GameObject> npc)
{
}

void AtkNPCGlobalState::Execute(std::shared_ptr<GameObject> npc)
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

void AtkNPCGlobalState::Exit(std::shared_ptr<GameObject> npc)
{
}
