#include "stdafx.h"
#include "SocketInit.h"

using namespace std;

SocketInit g_socketInit;

SocketInit::SocketInit()
{
#ifdef _WIN32
	// Windows������ WSAStartup, WSACleanup�� ���ʿ� ���Ŀ� �� �ѹ� �־�� �Ѵ�.
	WSADATA w;
	WSAStartup(MAKEWORD(2, 2), &w);
#endif


}

void SocketInit::Touch()
{
}

