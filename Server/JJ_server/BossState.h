#pragma once
#include "FSMState.h"
#include <chrono>
class GameObject;

class BossGlobalState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	bool is_invincible = false; // ������������ üũ�ϴ� ����

	std::chrono::time_point<std::chrono::system_clock> atk_delay_starttime;
	bool is_atkdelay = false;
public:
	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);

	virtual void SetInvincible() {
		is_invincible = true;
		starttime = std::chrono::system_clock::now(); // �������� ���۽ð�
	}
	virtual bool GetInvincible() const { return is_invincible; } // ������������ üũ�ϴ� �Լ�

	virtual void SetAtkDelay() {
		is_atkdelay = true;
		atk_delay_starttime = std::chrono::system_clock::now(); // ���� ������ ���۽ð�
	}
	virtual bool GetAtkDelay() const { return is_atkdelay; } // ���� ������ �������� üũ�ϴ� �Լ�

};

class BossStandingState : public FSMState<GameObject>
{
private:
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time{};
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class BossMoveState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time{};
	char move_type{};	// 0 ���� 1 ȸ���ϸ鼭 ���� 2 ȸ��
	char rotate_type{}; // 0 �ð���� 1 �ݽð����
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class BossChaseState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time{};
	long long aggro_player_id = -1; // ������ �÷��̾� ID
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class BossAttackState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	static int Sp_atk_counter;
	long long duration_time{};
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class BossDieState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time{};
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class BossRespawnState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time{};
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class BossHitState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time{};
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};