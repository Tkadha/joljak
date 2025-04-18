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
	char move_type;	// 0 ���� 1 ȸ���ϸ鼭 ���� 2 ȸ��
	char rotate_type; // 0 �ð���� 1 �ݽð����
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

