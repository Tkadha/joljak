#include "stdafx.h"
#ifdef _WIN32
#include <rpc.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#include "Socket.h"
#include "Endpoint.h"
#include "SocketInit.h"
#include "Exception.h"



using namespace std;

std::string GetLastErrorAsString();

Socket::Socket(SocketType socketType)
{
	g_socketInit.Touch();

	if(socketType==SocketType::Tcp)
	{
#ifdef _WIN32
		m_fd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
		m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
	}
	else 
	{
#ifdef _WIN32
		m_fd = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
		m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
	}

#ifdef _WIN32
	ZeroMemory(&m_readOverlappedStruct, sizeof(m_readOverlappedStruct));
#endif
}

Socket::Socket(SOCKET fd)
{
	g_socketInit.Touch();
	m_fd = fd;

#ifdef _WIN32
	ZeroMemory(&m_readOverlappedStruct, sizeof(m_readOverlappedStruct));
#endif
}

Socket::Socket()
{
#ifdef _WIN32
	static_assert(-1 == INVALID_SOCKET, "");
#endif

	m_fd = -1;

#ifdef _WIN32
	ZeroMemory(&m_readOverlappedStruct, sizeof(m_readOverlappedStruct));
#endif
}

Socket::~Socket()
{
	Close();
}

void Socket::Bind(const Endpoint& endpoint)
{
	if (bind(m_fd, (sockaddr*)&endpoint.m_ipv4Endpoint, sizeof(endpoint.m_ipv4Endpoint)) < 0)
	{
		stringstream ss;
		ss << "bind failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
}

void Socket::Connect(const Endpoint& endpoint)
{
	if (connect(m_fd, (sockaddr*)&endpoint.m_ipv4Endpoint, sizeof(endpoint.m_ipv4Endpoint)) < 0)
	{
		stringstream ss;
		ss << "connect failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
}

int Socket::Send(const char* data, int length)
{
	return ::send(m_fd, data, length, 0);
}

void Socket::SendOverlapped(void* packet)
{
	OVER_EXP* data = new OVER_EXP(reinterpret_cast<char*>(packet));
	WSASend(m_fd, &data->wsabuf, 1, 0, 0, &data->over, 0);
}

void Socket::Close()
{
#ifdef _WIN32
	closesocket(m_fd);
#else
	close(m_fd);
#endif
}

void Socket::Listen()
{
	listen(m_fd, SOMAXCONN);
}

int Socket::Accept(Socket& acceptedSocket, string& errorText)
{
	acceptedSocket.m_fd = accept(m_fd, NULL, 0);
	if (acceptedSocket.m_fd == -1)
	{
		errorText = GetLastErrorAsString();
		return -1;
	}
	else
		return 0;
}

#ifdef _WIN32

bool Socket::AcceptOverlapped(Socket& acceptCandidateSocket, string& errorText)
{
	if (AcceptEx == NULL)
	{
		DWORD bytes;
		UUID uuid{ UUID(WSAID_ACCEPTEX) };
		WSAIoctl(m_fd,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&uuid,
			sizeof(UUID),
			&AcceptEx,
			sizeof(AcceptEx),
			&bytes,
			NULL,
			NULL);

		if (AcceptEx == NULL)
		{
			throw Exception("Getting AcceptEx ptr failed.");
		}
	}


	char ignored[200];
	DWORD ignored2 = 0;

	bool ret = AcceptEx(m_fd,
		acceptCandidateSocket.m_fd,
		&ignored,
		0,
		50,
		50,
		&ignored2,
		&m_readOverlappedStruct
	) == TRUE;
	
	return ret;
}


int Socket::UpdateAcceptContext(Socket& listenSocket)
{
	sockaddr_in ignore1;
	sockaddr_in ignore3;
	INT ignore2,ignore4;

	char ignore[1000];
	GetAcceptExSockaddrs(ignore,
		0,
		50,
		50,
		(sockaddr**)&ignore1,
		&ignore2,
		(sockaddr**)&ignore3,
		&ignore4);

	return setsockopt(m_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSocket.m_fd, sizeof(listenSocket.m_fd));
}

#endif // _WIN32

Endpoint Socket::GetPeerAddr()
{
	Endpoint ret;
	socklen_t retLength = sizeof(ret.m_ipv4Endpoint);
	if (::getpeername(m_fd, (sockaddr*)&ret.m_ipv4Endpoint, &retLength) < 0)
	{
		stringstream ss;
		ss << "getPeerAddr failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
	if(retLength > sizeof(ret.m_ipv4Endpoint))
	{
		stringstream ss;
		ss << "getPeerAddr buffer overrun: " << retLength;
		throw Exception(ss.str().c_str());
	}

	return ret;
}

int Socket::Receive()
{
	return (int)recv(m_fd, m_recv_over.send_buf, BUFSIZE, 0);
}

#ifdef _WIN32

int Socket::ReceiveOverlapped()
{

	m_readFlags = 0;
	memset(&m_recv_over.over, 0, sizeof(m_recv_over.over));
	m_recv_over.wsabuf.len = BUFSIZE - m_prev_remain;
	m_recv_over.wsabuf.buf = m_recv_over.send_buf + m_prev_remain;

	return WSARecv(m_fd, &m_recv_over.wsabuf, 1, NULL, &m_readFlags, &m_recv_over.over, NULL);
}

#endif

void Socket::SetNonblocking()
{
	u_long val = 1;
#ifdef _WIN32
	int ret = ioctlsocket(m_fd, FIONBIO, &val);
#else
	int ret = ioctl(m_fd, FIONBIO, &val);
#endif
	if (ret != 0)
	{
		stringstream ss;
		ss << "bind failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
}

std::string GetLastErrorAsString()
{
#ifdef _WIN32
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

#else 
	std::string message = strerror(errno);
#endif
	return message;
}
