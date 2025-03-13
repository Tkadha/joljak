#include "Object.h"

void Object::move(std::pair<char, char> dir)
{
	x += dir.first;
	y += dir.second;
}
