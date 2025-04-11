#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <queue>
#include <mutex>
#include <memory>
#pragma comment (lib, "WS2_32.LIB")
#include "../../Server/Global.h"

#define BUFSIZE 1024 // 패킷(현재는 버퍼)크기


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
	queue<pair<unique_ptr<char[]> , short >> send_queue;
	mutex mu;

public:
	NetworkManager() = default;
	~NetworkManager() = default;
	static NetworkManager& GetInstance();

	void Init();
	void do_recv();
	void do_send(const char* packet, short size);

	void Process_Packet(char* packet);
	void ReconnectToNewServer(const char* n_addr, short n_port);

	template<class P>
	void PushQueue(P packet, short size)
	{
		auto buffer = make_unique<char[]>(size);
		memcpy(buffer.get(), reinterpret_cast<const char*>(& packet), size);
		lock_guard<mutex> lock(mu);
		send_queue.push({ move(buffer),size });
	}
	pair<unique_ptr<char[]>, short> PopQueue()
	{
		lock_guard<mutex> lock(mu);
		auto p = move(send_queue.front());
		send_queue.pop();
		return p;
	}
};

