#pragma once
#include <memory>

template <class entity_type>
class FSMState
{
public:
	virtual ~FSMState() {}

	virtual void Enter(std::shared_ptr<entity_type>) = 0;
	virtual void Execute(std::shared_ptr<entity_type>) = 0;
	virtual void Exit(std::shared_ptr<entity_type>) = 0;
};

