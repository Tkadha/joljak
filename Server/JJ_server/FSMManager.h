#pragma once
#include "FSMState.h"
#include <memory>
template <class entity_type>
class FSMManager
{
	std::shared_ptr<entity_type> Owner;
	std::shared_ptr<FSMState<entity_type>> Currentstate;
	std::shared_ptr<FSMState<entity_type>> Globalstate;
public:
	FSMManager(entity_type* owner) : Owner(owner), Currentstate(NULL), Globalstate(NULL) {}
	virtual ~FSMManager() {}

	FSMManager(const FSMManager&) = delete;
	FSMManager& operator=(const FSMManager&) = delete;

	void SetCurrentState(std::shared_ptr<FSMState<entity_type>> s) { 
		Currentstate = s; }
	void SetGlobalState(std::shared_ptr<FSMState<entity_type>> s) { 
		Globalstate = s; }

	void Update()const
	{
		if (Globalstate) Globalstate->Execute(Owner);
		if (Currentstate) Currentstate->Execute(Owner);
	}

	void ChangeState(std::shared_ptr<FSMState<entity_type>> newstate)
	{
		Currentstate->Exit(Owner);
		Currentstate = newstate;
		Currentstate->Enter(Owner);
	}

	std::shared_ptr<FSMState<entity_type>> GetCurrentState()  const { 
		return Currentstate; }
	void SetInvincible() { 
		Globalstate->SetInvincible(); }
	bool GetInvincible() const { 
		return Globalstate->GetInvincible(); }
	void SetAtkDelay() { 
		Globalstate->SetAtkDelay(); }
	bool GetAtkDelay() const { 
		return Globalstate->GetAtkDelay(); }
};