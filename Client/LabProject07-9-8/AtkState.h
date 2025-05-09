#pragma once
#include "FSMState.h"
#include <chrono>
class CGameObject;

class AtkNPCGlobalState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	bool is_invincible = false; // 무적상태인지 체크하는 변수
	public:
	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);

	virtual void SetInvincible() {
		is_invincible = true;
		starttime = std::chrono::system_clock::now(); // 무적상태 시작시간
	} // 무적상태로 전환
	virtual bool GetInvincible() const { return is_invincible; } // 무적상태인지 체크하는 함수
};




class AtkNPCStandingState : public FSMState<CGameObject>
{
private:
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

class AtkNPCMoveState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
	char move_type;	// 0 전진 1 회전하면서 전진 2 회전
	char rotate_type; // 0 시계방향 1 반시계방향
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

class AtkNPCChaseState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

class AtkNPCAttackState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

class AtkNPCDieState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

class AtkNPCRespawnState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

class AtkNPCHitState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};