#pragma once
#include "FSMState.h"
#include <chrono>
class GameObject;

class BossGlobalState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	bool is_invincible = false; // 무적상태인지 체크하는 변수

	std::chrono::time_point<std::chrono::system_clock> atk_delay_starttime;
	bool is_atkdelay = false;
public:
	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);

	virtual void SetInvincible() {
		is_invincible = true;
		starttime = std::chrono::system_clock::now(); // 무적상태 시작시간
	}
	virtual bool GetInvincible() const { return is_invincible; } // 무적상태인지 체크하는 함수

	virtual void SetAtkDelay() {
		is_atkdelay = true;
		atk_delay_starttime = std::chrono::system_clock::now(); // 공격 딜레이 시작시간
	}
	virtual bool GetAtkDelay() const { return is_atkdelay; } // 공격 딜레이 상태인지 체크하는 함수

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
	char move_type{};	// 0 전진 1 회전하면서 전진 2 회전
	char rotate_type{}; // 0 시계방향 1 반시계방향
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
	long long aggro_player_id = -1; // 추적할 플레이어 ID
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