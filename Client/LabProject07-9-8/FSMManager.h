#pragma once
#include "FSMState.h"
#include <memory>
template <class entity_type>
class FSMManager
{
	std::shared_ptr<entity_type> Owner;
	std::shared_ptr<FSMState<entity_type>> Currentstate;
	
public:
	FSMManager(entity_type* owner) : Owner(owner), Currentstate(NULL) {}
	virtual ~FSMManager() {}

	FSMManager(const FSMManager&) = delete;
	FSMManager& operator=(const FSMManager&) = delete;

	void SetCurrentState(std::shared_ptr<FSMState<entity_type>> s) { Currentstate = s; }

	void Update()const
	{
		if (Currentstate) Currentstate->Execute(Owner);
	}

	void ChangeState(std::shared_ptr<FSMState<entity_type>> newstate)
	{
		Currentstate->Exit(Owner);
		Currentstate = newstate;
		Currentstate->Enter(Owner);
	}

	std::shared_ptr<FSMState<entity_type>> CurrentState()  const { return Currentstate; }
};

