#pragma once
#include <random>

extern std::random_device rd;
extern std::default_random_engine dre;
extern std::uniform_int_distribution<int> rand_time;
extern std::uniform_int_distribution<int> rand_type;

extern std::mt19937 gen;