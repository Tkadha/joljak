#include "PigState.h"
#include "Pig.h"
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

void PigState::Enter(std::shared_ptr<Pig> pig)
{
	std::cout << "Enter PigState" << std::endl;
}

void PigState::Execute(std::shared_ptr<Pig> pig)
{
	std::cout << "Execute PigState\n" << std::endl;
	std::cout << "before: " << pig->x << ", " << pig->y << std::endl;
	pig->move(way[rand_dirUid(dre)]);
	std::cout << "after: " << pig->x << ", " << pig->y << std::endl;
}

void PigState::Exit(std::shared_ptr<Pig> pig)
{
	std::cout << "Exit PigState" << std::endl;
}



std::shared_ptr<PigattackState> PigattackState::Instance()
{
	static std::shared_ptr<PigattackState> instance = std::make_shared<PigattackState>();

	return instance;
}

void PigattackState::Enter(std::shared_ptr<Pig> pig)
{
	std::cout << "Enter PigattackState" << std::endl;
}

void PigattackState::Execute(std::shared_ptr<Pig> pig)
{
	std::cout << "Execute PigattackState\n" << std::endl;
}

void PigattackState::Exit(std::shared_ptr<Pig> pig)
{
	std::cout << "Exit PigattackState" << std::endl;
}
