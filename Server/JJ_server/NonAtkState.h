#pragma once
#include "FSMState.h"
#include <chrono>
class GameObject;

class NonAtkNPCGlobalState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	bool is_invincible = false; // ������������ üũ�ϴ� ����
	long long sustainment_time = 1500.f;
public:
	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);

	virtual void SetInvincible(long long time = 1500.f) {
		is_invincible = true;
		sustainment_time = time; // �������� ���ӽð� ����
		starttime = std::chrono::system_clock::now(); // �������� ���۽ð�
	} // �������·� ��ȯ
	virtual bool GetInvincible() const { return is_invincible; } // ������������ üũ�ϴ� �Լ�
};

class NonAtkNPCStandingState : public FSMState<GameObject>
{
private:
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class NonAtkNPCMoveState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
	char move_type;	// 0 ���� 1 ȸ���ϸ鼭 ���� 2 ȸ��
	char rotate_type; // 0 �ð���� 1 �ݽð����
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class NonAtkNPCRunAwayState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
	long long total_time;
	char move_type;	// 0 ���� 1 ȸ���ϸ鼭 ����
	char rotate_type; // 0 �ð���� 1 �ݽð����
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class NonAtkNPCDieState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};

class NonAtkNPCRespawnState : public FSMState<GameObject>
{
	std::chrono::time_point<std::chrono::system_clock> starttime;
	std::chrono::time_point<std::chrono::system_clock> endtime;
	long long duration_time;
public:

	virtual void Enter(std::shared_ptr<GameObject> npc);

	virtual void Execute(std::shared_ptr<GameObject> npc);

	virtual void Exit(std::shared_ptr<GameObject> npc);
};