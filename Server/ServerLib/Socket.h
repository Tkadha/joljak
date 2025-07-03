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
// SOCKET�� 64bit ȯ�濡�� 64bit�̴�. �ݸ� linux������ ������ 32bit�̴�. �� ���̸� ����.
typedef int SOCKET;
#endif

#define BUFSIZE 1024 // ��Ŷ(����� ����)ũ��

//#define MAX_CLIENT 100 // �ִ� ���Ӱ����� Ŭ���̾�Ʈ ��
//#define MAX_WORKERTHREAD 4 // ������ Ǯ(CP��ü)�� ���� ������ ��

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

// ���� Ŭ����
class Socket
{
public:
	SOCKET m_fd; // ���� �ڵ�

#ifdef _WIN32
	// AcceptEx �Լ� ������
	LPFN_ACCEPTEX AcceptEx = NULL;
	// Overlapped I/O�� IOCP�� �� ������ ���˴ϴ�. ���� overlapped I/O ���̸� true�Դϴ�.
	bool m_isReadOverlapped = false;

	// Overlapped receive or accept�� �� �� ���Ǵ� overlapped ��ü�Դϴ�. 
	// I/O �Ϸ� �������� �����Ǿ�� �մϴ�.
	WSAOVERLAPPED m_readOverlappedStruct;
#endif
	// Receive�� ReceiveOverlapped�� ���� ���ŵǴ� �����Ͱ� ä������ ���Դϴ�.
	OVER_EXP m_recv_over = OVER_EXP();
	u_short m_prev_remain = 0;

#ifdef _WIN32
	// overlapped ������ �ϴ� ���� ���⿡ recv�� flags�� ���ϴ� ���� ä�����ϴ�.
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
