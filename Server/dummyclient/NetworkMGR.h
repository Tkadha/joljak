#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <list>
#include <mutex>
#include <string>

#include "../Global.h"

#ifdef _DEBUG
#pragma comment(lib,"../x64/Debug/serverlib.lib")
#else
#pragma comment(lib,"../x64/Release/serverlib.lib")
#endif
#include "../JJ_server/ServerLib/ServerHeader.h"

using namespace std;

class Socket;
#define PORT 9000

class NetworkMGR
{
	recursive_mutex mutex;

public:
	char* SERVERIP;
	shared_ptr<Socket> tcpSocket;

	int id;
	string name;
	bool is_mage;

	bool b_isNet; // ���� ���� ����
	bool b_isLogin; // �α��� �Ϸ� ����
	bool b_isLoginProg; // �α��� ���� ������ ����
private:
	NetworkMGR() = default;
	NetworkMGR(const NetworkMGR&) = delete;
	NetworkMGR& operator=(const NetworkMGR&) = delete;
	NetworkMGR(NetworkMGR&&) = delete;
	NetworkMGR& operator=(NetworkMGR&&) = delete;
	~NetworkMGR() = default;
public:
	static NetworkMGR& GetInstance();
	void Initialize();
	void Update();

	void do_connetion();
	void do_recv();
	void do_send(const char* buf, short buf_size);
	void send_chat_packet(string str);
	void Process_Packet(char* p_Packet);
};