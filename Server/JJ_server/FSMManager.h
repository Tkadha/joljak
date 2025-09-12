#pragma once
#include "FSMState.h"
#include <memory>
template <class entity_type>
class FSMManager
{
	std::weak_ptr<entity_type> Owner;
	std::shared_ptr<FSMState<entity_type>> Currentstate;
	std::shared_ptr<FSMState<entity_type>> Globalstate;
public:
	FSMManager(std::shared_ptr<entity_type> owner) 
		: Owner(owner), Currentstate(NULL), Globalstate(NULL) {}
	virtual ~FSMManager() {}

	FSMManager(const FSMManager&) = delete;
	FSMManager& operator=(const FSMManager&) = delete;

	void SetCurrentState(std::shared_ptr<FSMState<entity_type>> s) { 
		Currentstate = s; }
	void SetGlobalState(std::shared_ptr<FSMState<entity_type>> s) { 
		Globalstate = s; }

	void Update()const
	{
		if (auto owner_sp = Owner.lock()) {
			if (Globalstate) Globalstate->Execute(owner_sp);
			if (Currentstate) Currentstate->Execute(owner_sp);
		}
	}

	void ChangeState(std::shared_ptr<FSMState<entity_type>> newstate)
	{
		if (auto owner_sp = Owner.lock()) {
			Currentstate->Exit(owner_sp);
			Currentstate = newstate;
			Currentstate->Enter(owner_sp);
		}
	}

	std::shared_ptr<FSMState<entity_type>> GetCurrentState()  const { 
		return Currentstate; }
	void SetInvincible(long long time = 1500.f) { 
		Globalstate->SetInvincible(time); }
	bool GetInvincible() const { 
		return Globalstate->GetInvincible(); }
	void SetAtkDelay() { 
		Globalstate->SetAtkDelay(); }
	bool GetAtkDelay() const { 
		return Globalstate->GetAtkDelay(); }
};