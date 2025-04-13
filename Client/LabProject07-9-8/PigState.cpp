#include "PigState.h"
#include "Object.h"
#include <iostream>
#include <random>

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution<int> rand_dirUid(0, 3);

std::pair<char, char> way[] = { {1,0}, {-1,0}, {0,1},{0,-1} };

std::shared_ptr<PigState> PigState::Instance()
{
	static std::shared_ptr<PigState> instance = std::make_shared<PigState>();

	return instance;
}

void PigState::Enter(std::shared_ptr<CGameObject> pig)
{
	std::cout << "Enter PigState" << std::endl;
}

void PigState::Execute(std::shared_ptr<CGameObject> pig)
{
	auto obj = dynamic_cast<CMonsterObject*>(pig.get());
	std::cout << "Execute PigState\n" << std::endl;
	int dir = rand_dirUid(dre);
	pig->Move(XMFLOAT3(way[dir].first, 0, way[dir].second));
}

void PigState::Exit(std::shared_ptr<CGameObject> pig)
{
	std::cout << "Exit PigState" << std::endl;
}



std::shared_ptr<PigattackState> PigattackState::Instance()
{
	static std::shared_ptr<PigattackState> instance = std::make_shared<PigattackState>();

	return instance;
}

void PigattackState::Enter(std::shared_ptr<CGameObject> pig)
{
	std::cout << "Enter PigattackState" << std::endl;
}

void PigattackState::Execute(std::shared_ptr<CGameObject> pig)
{
	std::cout << "Execute PigattackState\n" << std::endl;
}

void PigattackState::Exit(std::shared_ptr<CGameObject> pig)
{
	std::cout << "Exit PigattackState" << std::endl;
}
