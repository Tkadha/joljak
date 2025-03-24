#pragma once
#include "Object.h"
#include "FSMManager.h"
#include "PigState.h"

class Pig : public Object
{
	FSMManager<Pig>* FSM_manager;

public:
	Pig()
	{
		FSM_manager = new FSMManager<Pig>(this);
		FSM_manager->SetCurrentState(PigState::Instance());
	}

	void Update()
	{
		FSM_manager->Update();
	}
	void ChangeState(std::shared_ptr<FSMState<Pig>> newstate)
	{
		FSM_manager->ChangeState(newstate);
	}
};

