#pragma once
#include "FSMState.h"
#include <memory>
class Pig;

class PigState : public FSMState<Pig>
{

public:
	static std::shared_ptr<PigState> Instance();

	virtual void Enter(std::shared_ptr<Pig> pig);

	virtual void Execute(std::shared_ptr<Pig> pig);

	virtual void Exit(std::shared_ptr<Pig> pig);
};


class PigattackState : public FSMState<Pig>
{

public:
	static std::shared_ptr<PigattackState> Instance();

	virtual void Enter(std::shared_ptr<Pig> pig);

	virtual void Execute(std::shared_ptr<Pig> pig);

	virtual void Exit(std::shared_ptr<Pig> pig);
};

