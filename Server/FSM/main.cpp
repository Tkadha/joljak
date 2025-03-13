#include "Pig.h"
#include "FSMState.h"
#include "PigState.h"
#include <Windows.h>

using namespace std;


int main()
{
	Pig pig;

	for (int i = 0; i < 5; ++i) {
		pig.Update();
		Sleep(500);
	}
	pig.ChangeState(PigattackState::Instance());
	for (int i = 0; i < 5; ++i) {
		pig.Update();
		Sleep(500);
	}
	return 0;
}
