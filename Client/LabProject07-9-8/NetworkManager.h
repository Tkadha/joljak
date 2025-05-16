#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <queue>
#include <mutex>
#include <memory>
#pragma comment (lib, "WS2_32.LIB")
#include "../../Server/Global.h"

#define BUFSIZE 4*1024*1024 // 패킷(현재는 버퍼)크기


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
	deque<pair<unique_ptr<char[]> , short >> send_queue;
	deque<pair<unique_ptr<char[]> , short >> recv_queue;
	mutex s_mu;
	mutex r_mu;

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
	void PushSendQueue(P packet, short size)
	{
		auto buffer = make_unique<char[]>(size);
		memcpy(buffer.get(), reinterpret_cast<const char*>(& packet), size);
		lock_guard<mutex> lock(s_mu);
		if (packet.type == static_cast<char>(E_PACKET::E_P_ROTATE))
		{
			for (auto it = send_queue.begin(); it != send_queue.end(); ++it)
			{
				if ((*it).second >= 1 && (*it).first[1] == static_cast<char>(E_PACKET::E_P_ROTATE))
				{
					send_queue.erase(it);
					break;
				}
			}
		}
		send_queue.push_back({ move(buffer),size });
	}
	pair<unique_ptr<char[]>, short> PopSendQueue()
	{
		lock_guard<mutex> lock(s_mu);
		auto p = move(send_queue.front());
		send_queue.pop_front();
		return p;
	}
	void PushRecvQueue(char* packet, short size)
	{
		auto buffer = make_unique<char[]>(size);
		memcpy(buffer.get(), packet, size);
		lock_guard<mutex> lock(r_mu);
		recv_queue.push_back({ move(buffer),size });
	}
	pair<unique_ptr<char[]>, short> PopRecvQueue()
	{
		lock_guard<mutex> lock(r_mu);
		auto p = move(recv_queue.front());
		recv_queue.pop_front();
		return p;
	}
};

