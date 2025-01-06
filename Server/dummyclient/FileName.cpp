#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")
#include "../Global.h"
#include "../JJ_server/ServerLib/ServerHeader.h"
#ifdef _DEBUG
#pragma comment(lib,"../x64/Debug/serverlib.lib")
#else
#pragma comment(lib,"../x64/Release/serverlib.lib")
#endif

constexpr short PORT = 9000;
constexpr char SERVER_ADDR[] = "127.0.0.1";

using namespace std;
bool bshutdown = false;
shared_ptr<Socket> server_s;
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void Process_Packet(char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_P_CHAT:
		CHAT_PACKET* recv_p = reinterpret_cast<CHAT_PACKET*>(packet);
		cout << recv_p->chat << endl;
		break;
	}
}

void read_n_send()
{
	std::cout << "Enter Message : ";
	string str;
	cin >> str;
	if (str.size() <= 1) {
		bshutdown = true;
		return;
	}
	CHAT_PACKET p;
	strcpy(p.chat, str.c_str());
	p.size = sizeof(CHAT_PACKET);
	p.type = static_cast<char>(E_PACKET::E_P_CHAT);

	OVER_EXP* send_over = new OVER_EXP(reinterpret_cast<char*>(&p), p.size);
	WSASend(server_s->m_fd, &send_over->wsabuf, 1, nullptr, 0, &send_over->over, send_callback);
}

void CALLBACK recv_callback(DWORD err, DWORD recv_size,
	LPWSAOVERLAPPED recv_over, DWORD sendflag)
{
	char* recv_buf = reinterpret_cast<OVER_EXP*>(recv_over)->send_buf;
	int recv_len = recv_size;
	{ // 패킷 수신
		int remain_data = recv_len + server_s->m_prev_remain;
		unsigned char packet_size = recv_buf[0];
		while (remain_data > 0 && packet_size <= remain_data) {
			if (packet_size == 0) {
				remain_data = 0;
				break;
			}
			// 패킷 처리
			Process_Packet(recv_buf);
			cout << " RECV!" << endl;

			// 다음 패킷 이동, 남은 데이터 갱신
			recv_buf += packet_size;
			remain_data -= packet_size;
		}
		// 남은 데이터 저장
		server_s->m_prev_remain = remain_data;

		// 남은 데이터가 0이 아닌 값을 가지면 recv_buf의 맨 앞으로 복사한다.
		if (remain_data > 0) {
			memcpy(server_s->m_recv_over.send_buf, recv_buf, remain_data);
		}
	}

	memset(server_s->m_recv_over.send_buf + server_s->m_prev_remain, 0,
		sizeof(server_s->m_recv_over.send_buf) - server_s->m_prev_remain);
	memset(&server_s->m_recv_over.wsabuf, 0, sizeof(server_s->m_recv_over.over));
	read_n_send();
}

void CALLBACK send_callback(DWORD err, DWORD sent_size,
	LPWSAOVERLAPPED send_over, DWORD sendflag)
{
	delete reinterpret_cast<OVER_EXP*>(send_over);

	server_s->m_readFlags = 0;
	ZeroMemory(&server_s->m_recv_over.wsabuf, sizeof(server_s->m_recv_over.wsabuf));
	server_s->m_recv_over.wsabuf.len = BUFSIZE - server_s->m_prev_remain;
	server_s->m_recv_over.wsabuf.buf = server_s->m_recv_over.send_buf + server_s->m_prev_remain;
	WSARecv(server_s->m_fd, &(server_s->m_recv_over.wsabuf), 1, nullptr, &server_s->m_readFlags, &(server_s->m_recv_over.over), recv_callback);
}

int main()
{
	std::wcout.imbue(std::locale("korean"));
	server_s = make_shared<Socket>(SocketType::Tcp);
	server_s->Connect(Endpoint("127.0.0.1", PORT));
	read_n_send();

	while (false == bshutdown) {
		SleepEx(0, TRUE);
	}
	WSACleanup();
}