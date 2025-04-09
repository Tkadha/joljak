#include "stdafx.h"
#include <iostream>
#ifdef _DEBUG
#pragma comment(lib,"../x64/Debug/serverlib.lib")
#else
#pragma comment(lib,"../x64/Release/serverlib.lib")
#endif
#include "../ServerLib/ServerHeader.h"
#include "../Global.h"
using namespace std;

#define LOBBY_PORT 8999
#define PORT 9000
#define DB_PORT 9001
//-----------------------------------------------
// 
//				game server
// 
//-----------------------------------------------


#define IOCPCOUNT 1

Iocp iocp(IOCPCOUNT); // �� ������ �����带 �� �ϳ��� ����. ���� ���⵵ 1�� ����.
shared_ptr<Socket> g_l_socket; // listensocket
shared_ptr<Socket> g_c_socket; // clientsocket
shared_ptr<RemoteClient>remoteClientCandidate;
vector<shared_ptr<thread>> worker_threads;

void ProcessClientLeave(shared_ptr<RemoteClient> remoteClient)
{
	// ���� Ȥ�� ���� �����̴�.
	// �ش� ������ �����ع�����. 
	remoteClient->tcpConnection.Close();
	RemoteClient::remoteClients.erase(remoteClient.get());

	cout << "Client left. There are " << RemoteClient::remoteClients.size() << " connections.\n";
}
void ProcessPacket(shared_ptr<RemoteClient>& client, char* packet);
void ProcessAccept();
void worker_thread()
{
	try {
		while (1)
		{
			// I/O �Ϸ� �̺�Ʈ�� ���� ������ ��ٸ��ϴ�.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// ���� �̺�Ʈ ������ ó���մϴ�.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				auto p_read_over = (OVER_EXP*)readEvent.lpOverlapped;

				if (COMP_TYPE::OP_SEND == p_read_over->comp_type) {

					p_read_over->m_isReadOverlapped = false;
					continue;
				}


				if (readEvent.lpCompletionKey == (ULONG_PTR)g_l_socket.get()) // ���������̸�
				{
					ProcessAccept();
				}
				else  // TCP ���� �����̸�
				{
					cout << "Recv!" << endl;
					// ó���� Ŭ���̾�Ʈ
					shared_ptr<RemoteClient> remoteClient;
					remoteClient = RemoteClient::remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					if (remoteClient)
					{
						// �̹� ���ŵ� �����̴�. ���� �Ϸ�� ���� �׳� ���� ����.
						remoteClient->tcpConnection.m_isReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0)
						{
							// ���� ����� 0 �� TCP ������ ������...
							// Ȥ�� ���� �� ���� ������ �� �����̴�...
							ProcessClientLeave(remoteClient);
						}
						else
						{
							// �̹� ���ŵ� �����̴�. ���� �Ϸ�� ���� �׳� ���� ����.
							char* recv_buf = remoteClient->tcpConnection.m_recv_over.send_buf;
							int recv_buf_length = ec;

							// ��Ŷ ������
							int remain_data = recv_buf_length + remoteClient->tcpConnection.m_prev_remain;
							u_char packet_size = recv_buf[0];
							while (remain_data > 0 && packet_size <= remain_data) {
								ProcessPacket(remoteClient, recv_buf);
								recv_buf += packet_size;
								remain_data -= packet_size;
							}
							remoteClient->tcpConnection.m_prev_remain = remain_data;
							if (remain_data > 0)
								memcpy(remoteClient->tcpConnection.m_recv_over.send_buf, recv_buf, remain_data);

							// �ٽ� ������ �������� overlapped I/O�� �ɾ�� �Ѵ�.
							if (remoteClient->tcpConnection.ReceiveOverlapped() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING)
							{
								ProcessClientLeave(remoteClient);
							}
							else
							{
								// I/O�� �ɾ���. �ϷḦ ����ϴ� �� ���·� �ٲ���.
								remoteClient->tcpConnection.m_isReadOverlapped = true;
							}
						}
					}
				}
			}
		}
	}
	catch (Exception& e) {
		cout << "Exception! " << e.what() << endl;
	}
}



int main(int argc, char* argv[])
{
	SetConsoleTitle(L"GameServer");
	try
	{
		g_l_socket = make_shared<Socket>(SocketType::Tcp);
		g_l_socket->Bind(Endpoint("0.0.0.0", PORT));
		g_l_socket->Listen();


		iocp.Add(*g_l_socket, g_l_socket.get());

		remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);

		string errorText;
		if (!g_l_socket->AcceptOverlapped(remoteClientCandidate->tcpConnection, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING) {
			throw Exception("Overlapped AcceptEx failed."s);
		}
		g_l_socket->m_isReadOverlapped = true;

		for (int i{}; i < IOCPCOUNT; ++i)
			worker_threads.emplace_back(make_shared<thread>(worker_thread));

		for (auto& th : worker_threads) th->join();

		g_l_socket->Close();
	}
	catch (Exception& e)
	{
		cout << "Exception! " << e.what() << endl;

	}
	return 0;
}

void ProcessPacket(shared_ptr<RemoteClient>& client, char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_P_CHAT:
	{
		CHAT_PACKET* r_packet = reinterpret_cast<CHAT_PACKET*>(packet);
		cout << client->m_id << " " << r_packet->chat << endl;
		CHAT_PACKET s_packet;
		s_packet.size = sizeof(CHAT_PACKET);
		s_packet.type = static_cast<unsigned char>(E_PACKET::E_P_CHAT);
		strcpy(s_packet.chat, r_packet->chat);
		//for (auto cl : RemoteClient::remoteClients) {
		//	if (cl.second != client) cl.second->tcpConnection.m_isReadOverlapped = false;
		//	cout << "Send: " << client->m_id << " to " << cl.second->m_id << endl;
		//	cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
		//	if (cl.second != client) cl.second->tcpConnection.m_isReadOverlapped = true;
		//}
		break;
	}
	default:
		break;
	}
}

void ProcessAccept()
{
	g_l_socket->m_isReadOverlapped = false;
	// �̹� accept�� �Ϸ�Ǿ���. ��������, Win32 AcceptEx ������ ������ ������ �۾��� ����. 
	if (remoteClientCandidate->tcpConnection.UpdateAcceptContext(*g_l_socket) != 0)
	{
		//���������� �ݾҴ��� �ϸ� ���⼭ �������Ŵ�. �׷��� �������� �Ҵɻ��·� ������.
		g_l_socket->Close();
	}
	else // �� ó����
	{
		shared_ptr<RemoteClient> remoteClient = remoteClientCandidate;
		cout << "accept - key: " << remoteClient.get() << endl;

		// �� TCP ���ϵ� IOCP�� �߰��Ѵ�.
		iocp.Add(remoteClient->tcpConnection, remoteClient.get());

		// overlapped ������ ���� �� �־�� �ϹǷ� ���⼭ I/O ���� ��û�� �ɾ�д�.
		if (remoteClient->tcpConnection.ReceiveOverlapped() != 0
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			// ����. ������ ��������. �׸��� �׳� ������.
			remoteClient->tcpConnection.Close();
		}
		else
		{
			// I/O�� �ɾ���. �ϷḦ ����ϴ� �� ���·� �ٲ���.
			remoteClient->tcpConnection.m_isReadOverlapped = true;

			// �� Ŭ���̾�Ʈ�� ��Ͽ� �߰�.
			remoteClient->m_id = reinterpret_cast<unsigned long long>(remoteClient.get());
			RemoteClient::remoteClients.insert({ remoteClient.get(), remoteClient });
			cout << "Client joined. There are " << RemoteClient::remoteClients.size() << " connections.\n";

		}

		// ����ؼ� ���� �ޱ⸦ �ؾ� �ϹǷ� �������ϵ� overlapped I/O�� ����.
		remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);
		string errorText;
		if (!g_l_socket->AcceptOverlapped(remoteClientCandidate->tcpConnection, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			// �������� �������� �Ҵ� ���·� ������. 
			g_l_socket->Close();
		}
		else
		{
			// ���������� ������ ������ ��ٸ��� ���°� �Ǿ���.
			g_l_socket->m_isReadOverlapped = true;
		}
	}
}

