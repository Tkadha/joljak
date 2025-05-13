#pragma once
#include <iostream>

class Object
{
public:
	Object() :x{ 0 }, y{ 0 } {}

	void move(std::pair<char, char>);

public:
	short x, y;
};

