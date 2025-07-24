#include "NetworkManager.h"
#include "../../Server/ServerLib/ServerHeader.h"
#ifdef _DEBUG
#pragma comment(lib,"../../Server/x64/Debug/serverlib.lib")
#else
#pragma comment(lib,"../../Server/x64/Release/serverlib.lib")
#endif

short PORT = 8999; // 완성결과는 const 제외 하나의 포트가 왔다갔다 하는 형식
const short GAME_PORT = 9000;
//char SERVER_ADDR[] = "58.228.11.233";
char SERVER_ADDR[] = "127.0.0.1";

void recv_callback(DWORD err, DWORD recv_size, LPWSAOVERLAPPED recv_over, DWORD sendflag)
{
	if (err != 0 || recv_size == 0) {
		//bshutdown = true;
		return;
	}

	char* recv_buf = reinterpret_cast<OVER_EXP*>(recv_over)->send_buf;
	int recv_len = recv_size;
	auto& nwManager = NetworkManager::GetInstance();
	{ // 패킷 수신
		int remain_data = recv_len + nwManager.server_s->m_prev_remain;
		int packet_size = recv_buf[0];
		while (remain_data > 0 && packet_size <= remain_data) {
			if (packet_size == 0) {
				remain_data = 0;
				break;
			}
			// 패킷 처리
			nwManager.Process_Packet(recv_buf);

			// 다음 패킷 이동, 남은 데이터 갱신
			recv_buf += packet_size;
			remain_data -= packet_size;
			packet_size = recv_buf[0];
		}
		// 남은 데이터 저장
		nwManager.server_s->m_prev_remain = remain_data;

		// 남은 데이터가 0이 아닌 값을 가지면 recv_buf의 맨 앞으로 복사한다.
		if (remain_data > 0) {
			memcpy(nwManager.server_s->m_recv_over.send_buf, recv_buf, remain_data);
		}
	}

	memset(nwManager.server_s->m_recv_over.send_buf + nwManager.server_s->m_prev_remain, 0,
		sizeof(nwManager.server_s->m_recv_over.send_buf) - nwManager.server_s->m_prev_remain);
	memset(&nwManager.server_s->m_recv_over.over, 0, sizeof(nwManager.server_s->m_recv_over.over));

	// 다음 수신 준비
	nwManager.do_recv();
}

void send_callback(DWORD err, DWORD sent_size, LPWSAOVERLAPPED send_over, DWORD sendflag)
{
	delete reinterpret_cast<OVER_EXP*>(send_over);
}


void NetworkManager::ReconnectToNewServer(const char* n_addr, short n_port)
{
	// 기존 소켓 종료
	closesocket(server_s->m_fd);
	Sleep(100);
	// 새로운 소켓 생성 및 연결
	char save_addr[20];
	strncpy(save_addr, n_addr, sizeof(save_addr));
	server_s = make_shared<Socket>(SocketType::Tcp);
	server_s->Connect(Endpoint(save_addr, n_port));
	//cout << "Reconnected to " << save_addr << ":" << n_port << endl;

	// 첫 번째 데이터 수신 시작
	do_recv();
}

NetworkManager& NetworkManager::GetInstance()
{
	static NetworkManager instance;
	return instance;
}

void NetworkManager::Init()
{
	server_s = make_shared<Socket>(SocketType::Tcp);
	server_s->Connect(Endpoint(SERVER_ADDR, GAME_PORT));
}

void NetworkManager::do_recv()
{
	server_s->m_readFlags = 0;
	server_s->m_recv_over.wsabuf.len = BUFSIZE - server_s->m_prev_remain;
	server_s->m_recv_over.wsabuf.buf = server_s->m_recv_over.send_buf + server_s->m_prev_remain;
	WSARecv(server_s->m_fd, &(server_s->m_recv_over.wsabuf), 1, nullptr, &server_s->m_readFlags, &(server_s->m_recv_over.over), recv_callback);
}

void NetworkManager::do_send(const char* packet, short size)
{
	OVER_EXP* send_over = new OVER_EXP(packet, size);
	WSASend(server_s->m_fd, &send_over->wsabuf, 1, nullptr, 0, &send_over->over, send_callback);
}

void NetworkManager::Process_Packet(char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_P_CHAT: {
		CHAT_PACKET* recv_p = reinterpret_cast<CHAT_PACKET*>(packet);
	}
						   break;
	case E_PACKET::E_P_CHANGEPORT: {
		CHANGEPORT_PACKET* recv_p = reinterpret_cast<CHANGEPORT_PACKET*>(packet);
		//cout << "Changing server to " << recv_p->addr << ":" << recv_p->port << endl;
		//ReconnectToNewServer(recv_p->addr, recv_p->port);
		//if (recv_p->port = 9000) s_type = SERVER_TYPE::E_GAME;
		//else s_type = SERVER_TYPE::E_LOBBY;
	}
								 break;
	case E_PACKET::E_DB_SUCCESS_FAIL: {
		DB_SUCCESS_FAIL_PACKET* recv_p = reinterpret_cast<DB_SUCCESS_FAIL_PACKET*>(packet);
		if (static_cast<E_PACKET>(recv_p->kind) == E_PACKET::E_DB_REGISTER) {
			if (recv_p->result == 1) {
				//cout << "Register Success" << endl;
			}
			else {
				//cout << "Register Fail" << endl;
			}
		}
		else if (static_cast<E_PACKET>(recv_p->kind) == E_PACKET::E_DB_LOGIN) {
			if (recv_p->result == 1) {
				//cout << "Login Success" << endl;
			}
			else {
				//cout << "Login Fail" << endl;
			}
		}
	}
		break;
	default:
		PushRecvQueue(packet, static_cast<short>(packet[0]));
		break;
	}
}

