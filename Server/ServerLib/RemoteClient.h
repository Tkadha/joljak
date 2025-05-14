#pragma once
#include <thread>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include "Socket.h"
using namespace std;



// TCP 연결 각각의 객체.
class RemoteClient
{
public:
	static unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

	Socket tcpConnection;		// accept한 TCP 연결
	ULONGLONG m_id;

	RemoteClient(): tcpConnection(SocketType::Tcp), m_id() {}
	RemoteClient(SocketType socketType) :tcpConnection(socketType) {}
};