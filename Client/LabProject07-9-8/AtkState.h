#pragma once
#include "FSMState.h"
#include <chrono>
class CGameObject;

class AtkNPCGlobalState : public FSMState<CGameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	bool is_invincible = false; // ������������ üũ�ϴ� ����
	public:
	virtual void Enter(std::shared_ptr<CGameObject> npc);

	virtual void Execute(std::shared_ptr<CGameObject> npc);

	virtual void Exit(std::shared_ptr<CGameObject> npc);

	virtual void SetInvincible() {
		is_invincible = true;
		starttime = std::chrono::system_clock::now(); // �������� ���۽ð�
	} // �������·� ��ȯ
	virtual bool GetInvincible() const { return is_invincible; } // ������������ üũ�ϴ� �Լ�
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
	char move_type;	// 0 ���� 1 ȸ���ϸ鼭 ���� 2 ȸ��
	char rotate_type; // 0 �ð���� 1 �ݽð����
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