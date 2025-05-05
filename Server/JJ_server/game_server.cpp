#include "stdafx.h"
#include <iostream>
#ifdef _DEBUG
#pragma comment(lib,"../x64/Debug/serverlib.lib")
#else
#pragma comment(lib,"../x64/Release/serverlib.lib")
#endif
#include "../ServerLib/ServerHeader.h"
#include "../Global.h"
#include "Player.h"
#include "Timer.h"
#include "Terrain.h"

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
const int iocpcount = std::thread::hardware_concurrency() - 1; // CPU �ھ� �� - 1

Iocp iocp(iocpcount); // �� ������ �����带 �� �ϳ��� ����. ���� ���⵵ 1�� ����.
shared_ptr<Socket> g_l_socket; // listensocket
shared_ptr<Socket> g_c_socket; // clientsocket
shared_ptr<PlayerClient>remoteClientCandidate;
vector<shared_ptr<thread>> worker_threads;
shared_ptr<thread> logic_thread;


Timer g_timer;
std::shared_ptr<Terrain> terrain = std::make_shared<Terrain>(_T("../../Client/LabProject07-9-8/Terrain/terrain_16.raw"), 2049, 2049, XMFLOAT3(5.f, 0.2f, 5.f));

void ProcessClientLeave(shared_ptr<PlayerClient> remoteClient)
{
	// ���� Ȥ�� ���� �����̴�.
	// �ش� ������ �����ع�����. 

	// �α׾ƿ� ���� ������
	for(auto& cl : PlayerClient::PlayerClients) {
		LOGOUT_PACKET s_packet;
		s_packet.size = sizeof(LOGOUT_PACKET);
		s_packet.type = static_cast<unsigned char>(E_PACKET::E_P_LOGOUT);
		s_packet.uid = remoteClient->m_id;
		cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
	}

	remoteClient->tcpConnection.Close();
	PlayerClient::PlayerClients.erase(remoteClient.get());

	cout << "Client left. There are " << PlayerClient::PlayerClients.size() << " connections.\n";
}
void ProcessPacket(shared_ptr<PlayerClient>& client, char* packet);
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
					shared_ptr<PlayerClient> remoteClient;
					remoteClient = PlayerClient::PlayerClients[(PlayerClient*)readEvent.lpCompletionKey];
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

void Logic_thread()
{
	g_timer.Start();
	while (true) {
		g_timer.Tick(120.f);
		for(auto& cl: PlayerClient::PlayerClients) {
			if (cl.second->GetDirection() != 0)
			{
				cl.second->Move(cl.second->GetDirection(), 100.f, true);
			}
			auto& beforepos = cl.second->GetPosition();
			cl.second->Update(g_timer.GetTimeElapsed());
			auto& pos = cl.second->GetPosition();
			if (beforepos.x != pos.x || beforepos.y != pos.y || beforepos.z != pos.z)
			{
				POSITION_PACKET s_packet;
				s_packet.size = sizeof(POSITION_PACKET);
				s_packet.type = static_cast<unsigned char>(E_PACKET::E_P_POSITION);
				s_packet.uid = cl.second->m_id;
				s_packet.position.x = pos.x;
				s_packet.position.y = pos.y;
				s_packet.position.z = pos.z;

				for (auto& client : PlayerClient::PlayerClients) {
					client.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
				}
			}
			//cout<< cl.second->m_id << " " << pos.x << " " << pos.y << " " << pos.z << endl;
		}
	}
}



void ProcessPacket(shared_ptr<PlayerClient>& client, char* packet)
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
	case E_PACKET::E_P_ROTATE:
	{
		ROTATE_PACKET* r_packet = reinterpret_cast<ROTATE_PACKET*>(packet);
		client->SetRight(XMFLOAT3{ r_packet->right.x,r_packet->right.y,r_packet->right.z });
		client->SetUp(XMFLOAT3{ r_packet->up.x,r_packet->up.y,r_packet->up.z });
		client->SetLook(XMFLOAT3{ r_packet->look.x,r_packet->look.y,r_packet->look.z });

		ROTATE_PACKET s_packet;
		s_packet.size = sizeof(ROTATE_PACKET);
		s_packet.type = static_cast<unsigned char>(E_PACKET::E_P_ROTATE);
		s_packet.right = r_packet->right;
		s_packet.up = r_packet->up;
		s_packet.look = r_packet->look;
		s_packet.uid = client->m_id;
		for(auto& cl : PlayerClient::PlayerClients) {
			cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
		}
	}
		break;
	case E_PACKET::E_P_INPUT:
	{
		INPUT_PACKET* r_packet = reinterpret_cast<INPUT_PACKET*>(packet);
		client->SetDirection(r_packet->direction);
		cout<< client->m_id << " " << r_packet->direction << endl;
		auto& pos = client->GetPosition();
		cout << client->m_id << " " << pos.x << " " << pos.y << " " << pos.z << endl;
		INPUT_PACKET s_packet;
		s_packet.size = sizeof(INPUT_PACKET);
		s_packet.type = static_cast<unsigned char>(E_PACKET::E_P_INPUT);
		s_packet.direction = r_packet->direction;
	}
	break;
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
		shared_ptr<PlayerClient> remoteClient = remoteClientCandidate;
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
			PlayerClient::PlayerClients.insert({ remoteClient.get(), remoteClient });
			cout << "Client joined. There are " << PlayerClient::PlayerClients.size() << " connections.\n";
			cout <<" Client id: "<< remoteClient->m_id << endl;
			remoteClient->SetTerrain(terrain);


			LOGIN_PACKET s_packet;
			s_packet.size = sizeof(LOGIN_PACKET);
			s_packet.type = static_cast<unsigned char>(E_PACKET::E_P_LOGIN);
			s_packet.uid = remoteClient->m_id;
			// �� ���� ������
			remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));


			// ������ �������� �÷��̾� ���� ������
			for (auto& cl : PlayerClient::PlayerClients) {
				if(cl.second.get() == remoteClient.get()) continue; // �� �ڽ��� �����Ѵ�.
				LOGIN_PACKET s_a_packet;
				s_a_packet.size = sizeof(LOGIN_PACKET);
				s_a_packet.type = static_cast<unsigned char>(E_PACKET::E_P_LOGIN);
				s_a_packet.uid = cl.second->m_id;
				remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_a_packet));

				// ��ġ�� ��������
				POSITION_PACKET s_p_packet;
				s_p_packet.size = sizeof(POSITION_PACKET);
				s_p_packet.type = static_cast<unsigned char>(E_PACKET::E_P_POSITION);
				s_p_packet.uid = cl.second->m_id;
				auto& pos = cl.second->GetPosition();
				s_p_packet.position.x = pos.x;
				s_p_packet.position.y = pos.y;
				s_p_packet.position.z = pos.z;
				remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_p_packet));

				// ȸ�� ������ ��������
				ROTATE_PACKET s_r_packet;
				s_r_packet.size = sizeof(ROTATE_PACKET);
				s_r_packet.type = static_cast<unsigned char>(E_PACKET::E_P_ROTATE);
				s_r_packet.uid = cl.second->m_id;
				auto& right = cl.second->GetRight();
				auto& up = cl.second->GetUp();
				auto& look = cl.second->GetLook();
				s_r_packet.right.x = right.x;
				s_r_packet.right.y = right.y;
				s_r_packet.right.z = right.z;
				s_r_packet.up.x = up.x;
				s_r_packet.up.y = up.y;
				s_r_packet.up.z = up.z;
				s_r_packet.look.x = look.x;
				s_r_packet.look.y = look.y;
				s_r_packet.look.z = look.z;
				remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_r_packet));
			}

			// ���� ���� ������
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second.get() == remoteClient.get()) continue; // �� �ڽ��� �����Ѵ�.
				cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
			}

		}

		// ����ؼ� ���� �ޱ⸦ �ؾ� �ϹǷ� �������ϵ� overlapped I/O�� ����.
		remoteClientCandidate = make_shared<PlayerClient>(SocketType::Tcp);
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

int main(int argc, char* argv[])
{
	SetConsoleTitle(L"GameServer");
	try
	{
		g_l_socket = make_shared<Socket>(SocketType::Tcp);
		g_l_socket->Bind(Endpoint("0.0.0.0", PORT));
		g_l_socket->Listen();


		iocp.Add(*g_l_socket, g_l_socket.get());

		remoteClientCandidate = make_shared<PlayerClient>(SocketType::Tcp);

		string errorText;
		if (!g_l_socket->AcceptOverlapped(remoteClientCandidate->tcpConnection, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING) {
			throw Exception("Overlapped AcceptEx failed."s);
		}
		g_l_socket->m_isReadOverlapped = true;

		for (int i{}; i < IOCPCOUNT; ++i)
			worker_threads.emplace_back(make_shared<thread>(worker_thread));
		logic_thread = make_shared<thread>(Logic_thread);

		for (auto& th : worker_threads) th->join();
		logic_thread->join();

		g_l_socket->Close();
	}
	catch (Exception& e)
	{
		cout << "Exception! " << e.what() << endl;

	}
	return 0;
}

