#pragma once
#include "FSMState.h"
#include <memory>
class CGameObject;

class PigState : public FSMState<CGameObject>
{

public:
	static std::shared_ptr<PigState> Instance();

	virtual void Enter(std::shared_ptr<CGameObject> pig);

	virtual void Execute(std::shared_ptr<CGameObject> pig);

	virtual void Exit(std::shared_ptr<CGameObject> pig);
};


class PigattackState : public FSMState<CGameObject>
{

public:
	static std::shared_ptr<PigattackState> Instance();

	virtual void Enter(std::shared_ptr<CGameObject> pig);

	virtual void Execute(std::shared_ptr<CGameObject> pig);

	virtual void Exit(std::shared_ptr<CGameObject> pig);
};

