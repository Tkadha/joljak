#pragma once

class Socket;
class IocpEvents;

// I/O Completion Port 객체.
class Iocp
{
public:
	// 1회의 GetQueuedCompletionStatus이 최대한 꺼내올 수 있는 일의 갯수
	static const int MaxEventCount = 1000;
	
	Iocp(int threadCount);
	~Iocp();

	void Add(Socket& socket, void* userPtr);
	
	HANDLE m_hIocp;
	int m_threadCount;
	void Wait(IocpEvents &output, int timeoutMs);
};

// IOCP의 GetQueuedCompletionStatus로 받은 I/O 완료신호들
class IocpEvents
{
public:
	// GetQueuedCompletionStatus으로 꺼내온 이벤트들
	OVERLAPPED_ENTRY m_events[Iocp::MaxEventCount];
	int m_eventCount;
};


