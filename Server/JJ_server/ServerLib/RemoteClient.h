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

	shared_ptr<thread> thread; // 클라이언트 처리를 하는 스레드 1개
	Socket tcpConnection;		// accept한 TCP 연결
	ULONGLONG m_id;

	RemoteClient(): thread(), tcpConnection(SocketType::Tcp), m_id() {}
	RemoteClient(SocketType socketType) :tcpConnection(socketType) {}
};

