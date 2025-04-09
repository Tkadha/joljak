#pragma once

#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")
#include "../../Server/Global.h"



using namespace std;

class Socket;

enum class SERVER_TYPE
{
	E_LOBBY = 0,
	E_GAME = 1
};

class NetworkManager
{
	public:
	shared_ptr<Socket> server_s;
	SERVER_TYPE s_type = SERVER_TYPE::E_LOBBY;
	NetworkManager() = default;
	~NetworkManager() = default;
	static NetworkManager& GetInstance();

	void Init();
	void do_recv();
	void do_send(const char* packet, short buf_size);

	void Process_Packet(char* packet);
	void ReconnectToNewServer(const char* n_addr, short n_port);

};

