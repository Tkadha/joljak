#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#else 
#include <sys/socket.h>
#endif


#include <string>

#ifndef _WIN32
// SOCKET은 64bit 환경에서 64bit이다. 반면 linux에서는 여전히 32bit이다. 이 차이를 위함.
typedef int SOCKET;
#endif

#define BUFSIZE 1024 // 패킷(현재는 버퍼)크기

//#define MAX_CLIENT 100 // 최대 접속가능한 클라이언트 수
//#define MAX_WORKERTHREAD 4 // 쓰레드 풀(CP객체)에 넣을 쓰레드 수

class Endpoint;

enum class SocketType
{
	Tcp,
	Udp,
};

enum class COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_FSM_UPDATE };
class OVER_EXP {
public:
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	char send_buf[BUFSIZE];
	COMP_TYPE comp_type;
	char m_isReadOverlapped = false;
	int obj_id;
	OVER_EXP() : comp_type(COMP_TYPE::OP_RECV)
	{
		ZeroMemory(&over, sizeof(over));
		wsabuf.len = BUFSIZE;
		wsabuf.buf = send_buf;
		ZeroMemory(&send_buf, sizeof(send_buf));
	}
	OVER_EXP(char* packet) : comp_type(COMP_TYPE::OP_SEND)
	{
		ZeroMemory(&over, sizeof(over));
		wsabuf.len = packet[0];
		wsabuf.buf = send_buf;
		ZeroMemory(&send_buf, sizeof(send_buf));
		memcpy(send_buf, packet, packet[0]);
	}
	OVER_EXP(const char* packet, short buf_size) : comp_type(COMP_TYPE::OP_SEND)
	{
		ZeroMemory(&over, sizeof(over));
		wsabuf.len = buf_size;
		wsabuf.buf = send_buf;
		ZeroMemory(&send_buf, sizeof(send_buf));
		memcpy(send_buf, packet, buf_size);
	}
	~OVER_EXP() {}
};

// 소켓 클래스
class Socket
{
public:
	SOCKET m_fd; // 소켓 핸들

#ifdef _WIN32
	// AcceptEx 함수 포인터
	LPFN_ACCEPTEX AcceptEx = NULL;
	// Overlapped I/O나 IOCP를 쓸 때에만 사용됩니다. 현재 overlapped I/O 중이면 true입니다.
	bool m_isReadOverlapped = false;

	// Overlapped receive or accept을 할 때 사용되는 overlapped 객체입니다. 
	// I/O 완료 전까지는 보존되어야 합니다.
	WSAOVERLAPPED m_readOverlappedStruct;
#endif
	// Receive나 ReceiveOverlapped에 의해 수신되는 데이터가 채워지는 곳입니다.
	OVER_EXP m_recv_over = OVER_EXP();
	u_short m_prev_remain = 0;

#ifdef _WIN32
	// overlapped 수신을 하는 동안 여기에 recv의 flags에 준하는 값이 채워집니다.
	DWORD m_readFlags = 0;
#endif

	Socket();
	Socket(SOCKET fd);
	Socket(SocketType socketType);
	~Socket();

	void Bind(const Endpoint& endpoint);
	void Connect(const Endpoint& endpoint);
	int Send(const char* data, int length);
	void SendOverlapped(void* packet);
	void Close();
	void Listen();
	int Accept(Socket& acceptedSocket, std::string& errorText);
#ifdef _WIN32
	bool AcceptOverlapped(Socket& acceptCandidateSocket, std::string& errorText);
	int UpdateAcceptContext(Socket& listenSocket);
#endif
	Endpoint GetPeerAddr();
	int Receive();
#ifdef _WIN32
	int ReceiveOverlapped();
#endif
	void SetNonblocking();
	
};

std::string GetLastErrorAsString();

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#endif
