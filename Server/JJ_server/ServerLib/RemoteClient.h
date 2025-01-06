#pragma once
#include <thread>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include "Socket.h"
using namespace std;



// TCP ���� ������ ��ü.
class RemoteClient
{
public:
	static unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

	shared_ptr<thread> thread; // Ŭ���̾�Ʈ ó���� �ϴ� ������ 1��
	Socket tcpConnection;		// accept�� TCP ����
	ULONGLONG m_id;

	RemoteClient(): thread(), tcpConnection(SocketType::Tcp), m_id() {}
	RemoteClient(SocketType socketType) :tcpConnection(socketType) {}
};

