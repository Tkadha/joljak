#pragma once
#include "FSMState.h"
#include <chrono>
class CGameObject;

class NonAtkNPCStandingState : public FSMState<CGameObject>
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

class NonAtkNPCMoveState : public FSMState<CGameObject>
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

class NonAtkNPCRunAwayState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);
};

