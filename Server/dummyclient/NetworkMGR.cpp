#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <string>

#include "NetworkMGR.h"
#pragma comment(lib, "WS2_32.LIB")

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	char* recv_buf = reinterpret_cast<OVER_EXP*>(recv_over)->send_buf;
	int recv_buf_Length = num_bytes;
	auto& netWorkMGR = NetworkMGR::GetInstance();
	{ // 패킷 수신
		int remain_data = recv_buf_Length + netWorkMGR.tcpSocket->m_prev_remain;
		unsigned char packet_size = recv_buf[0];
		while (remain_data > 0 && packet_size <= remain_data) {
			if (packet_size == 0) {
				remain_data = 0;
				break;
			}

			// 패킷 처리
			netWorkMGR.Process_Packet(recv_buf);
			cout << " RECV!" << endl;

			// 다음 패킷 이동, 남은 데이터 갱신
			recv_buf += packet_size;
			remain_data -= packet_size;
		}
		// 남은 데이터 저장
		netWorkMGR.tcpSocket->m_prev_remain = remain_data;

		// 남은 데이터가 0이 아닌 값을 가지면 recv_buf의 맨 앞으로 복사한다.
		if (remain_data > 0) {
			memcpy(netWorkMGR.tcpSocket->m_recv_over.send_buf, recv_buf, remain_data);
		}
	}

	memset(netWorkMGR.tcpSocket->m_recv_over.send_buf + netWorkMGR.tcpSocket->m_prev_remain, 0,
		sizeof(netWorkMGR.tcpSocket->m_recv_over.send_buf) - netWorkMGR.tcpSocket->m_prev_remain);
	memset(&netWorkMGR.tcpSocket->m_recv_over.over, 0, sizeof(netWorkMGR.tcpSocket->m_recv_over.over));
	netWorkMGR.do_recv();
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	delete reinterpret_cast<OVER_EXP*>(send_over);
}

NetworkMGR& NetworkMGR::GetInstance()
{
	static NetworkMGR instance;
	return instance;
}

void NetworkMGR::Initialize()
{
	tcpSocket = make_shared<Socket>(SocketType::Tcp);

	char isnet = 'n';
	std::cout << "서버 연결 여부(y/n) : ";
	std::cin >> isnet;

	if (isnet == 'n') {
		b_isNet = false;
		b_isLogin = true;
		return;
	}

	system("cls");
	//
	// ����
	//

	std::cout << std::endl << " ======== Login ======== " << std::endl << std::endl;

	std::cout << std::endl << "서버 주소 입력(ex 197.xxx.xxx.xxx) : " << std::endl;
	std::string server_s;
	std::cin >> server_s;
	SERVERIP = new char[server_s.size() + 1];
	SERVERIP[server_s.size()] = '\0';
	strcpy(SERVERIP, server_s.c_str());




	tcpSocket->Bind(Endpoint::Any);
	NetworkMGR::do_connetion();
}

void NetworkMGR::Update()
{
	SleepEx(0, true);
	NetworkMGR::do_recv();
}

void NetworkMGR::do_connetion() {
	tcpSocket->Connect(Endpoint(SERVERIP, PORT));
}

void NetworkMGR::do_recv() {
	tcpSocket->m_readFlags = 0;
	ZeroMemory(&tcpSocket->m_recv_over.over, sizeof(tcpSocket->m_recv_over.over));
	tcpSocket->m_recv_over.wsabuf.len = BUFSIZE - tcpSocket->m_prev_remain;
	tcpSocket->m_recv_over.wsabuf.buf = tcpSocket->m_recv_over.send_buf + tcpSocket->m_prev_remain;

	WSARecv(tcpSocket->m_fd, &(tcpSocket->m_recv_over.wsabuf), 1, 0, &tcpSocket->m_readFlags, &(tcpSocket->m_recv_over.over), recv_callback);
}

void NetworkMGR::do_send(const char* buf, short buf_size) {
	OVER_EXP* send_over = new OVER_EXP(buf, buf_size);
	WSASend(tcpSocket->m_fd, &send_over->wsabuf, 1, 0, 0, &send_over->over, send_callback);
}

void NetworkMGR::send_chat_packet(string str)
{
	CHAT_PACKET packet;
	packet.size = sizeof(CHAT_PACKET);
	packet.type = static_cast<unsigned char>(E_PACKET::E_P_CHAT);
	strcpy(packet.chat, str.c_str());
}


void NetworkMGR::Process_Packet(char* p_Packet)
{
	E_PACKET type = static_cast<E_PACKET>(p_Packet[1]);
	switch (type)
	{
	case E_PACKET::E_P_CHAT:
		CHAT_PACKET* recvPacket = reinterpret_cast<CHAT_PACKET*>(p_Packet);

		cout << " CHAT : " << recvPacket->chat << endl;
		break;
	}
}
