#include "stdafx.h"
#include "RandomUtil.h"

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution<int> rand_time(3, 6);
std::uniform_int_distribution<int> rand_type(0, 2);
std::mt19937 gen(rd());