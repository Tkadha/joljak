#include <iostream>
#ifdef _DEBUG
#pragma comment(lib,"../x64/Debug/serverlib.lib")
#else
#pragma comment(lib,"../x64/Release/serverlib.lib")
#endif
#include "../ServerLib/ServerHeader.h"
#include "../Global.h"

#include "DBManager.h"
using namespace std;

#define LOBBY_PORT 8999
#define GAME_PORT 9000
#define PORT 9001

DBManager dbm;
char gameserver_addr[] = "127.0.0.1";
//------------------------------------
// 
//			database Server
// 
//------------------------------------

#define IOCPCOUNT 1

Iocp iocp(IOCPCOUNT); // 본 예제는 스레드를 딱 하나만 쓴다. 따라서 여기도 1이 들어간다.
shared_ptr<Socket> g_l_socket; // listensocket
shared_ptr<Socket> g_c_socket; // clientsocket
shared_ptr<RemoteClient>remoteClientCandidate;
vector<shared_ptr<thread>> worker_threads;

void ProcessClientLeave(shared_ptr<RemoteClient> remoteClient)
{
	// 에러 혹은 소켓 종료이다.
	// 해당 소켓은 제거해버리자. 
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
			// I/O 완료 이벤트가 있을 때까지 기다립니다.
			LobbyIocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// 받은 이벤트 각각을 처리합니다.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				auto p_read_over = (OVER_EXP*)readEvent.lpOverlapped;

				if (COMP_TYPE::OP_SEND == p_read_over->comp_type) {

					p_read_over->m_isReadOverlapped = false;
					continue;
				}


				if (readEvent.lpCompletionKey == (ULONG_PTR)g_l_socket.get()) // 리슨소켓이면
				{
					ProcessAccept();
				}
				else  // TCP 연결 소켓이면
				{
					cout << "Recv!" << endl;
					// 처리할 클라이언트
					shared_ptr<RemoteClient> remoteClient;
					remoteClient = RemoteClient::remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					if (remoteClient)
					{
						// 이미 수신된 상태이다. 수신 완료된 것을 그냥 꺼내 쓰자.
						remoteClient->tcpConnection.m_isReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0)
						{
							// 읽은 결과가 0 즉 TCP 연결이 끝났다...
							// 혹은 음수 즉 뭔가 에러가 난 상태이다...
							ProcessClientLeave(remoteClient);
						}
						else
						{
							// 이미 수신된 상태이다. 수신 완료된 것을 그냥 꺼내 쓰자.
							char* recv_buf = remoteClient->tcpConnection.m_recv_over.send_buf;
							int recv_buf_length = ec;

							// 패킷 재조립
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

							// 다시 수신을 받으려면 overlapped I/O를 걸어야 한다.
							if (remoteClient->tcpConnection.ReceiveOverlapped() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING)
							{
								ProcessClientLeave(remoteClient);
							}
							else
							{
								// I/O를 걸었다. 완료를 대기하는 중 상태로 바꾸자.
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
	SetConsoleTitle(L"DataBaseServer");
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
//
//int main()
//{
//	if(dbm.Login("test", "test")) printf("로그인 성공\n");
//	else printf("로그인 실패\n");
//}

void ProcessPacket(shared_ptr<RemoteClient>& client, char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_DB_REGISTER:	// 회원가입
	{
		DB_REGISTER_PACKET* db_register_packet = (DB_REGISTER_PACKET*)packet;
		if (dbm.Register(db_register_packet->id, db_register_packet->pw)) {	// 회원가입 성공
			DB_SUCCESS_FAIL_PACKET sf_packet;
			sf_packet.kind = static_cast<char>(E_PACKET::E_DB_REGISTER);
			sf_packet.result = 1;	// 성공
			sf_packet.uid = db_register_packet->uid;
			client->tcpConnection.Send((char*)&sf_packet, sizeof(DB_SUCCESS_FAIL_PACKET));
		}
		else {	// 회원가입 실패
			DB_SUCCESS_FAIL_PACKET sf_packet;
			sf_packet.kind = static_cast<char>(E_PACKET::E_DB_REGISTER);
			sf_packet.result = 0;	// 실패
			sf_packet.uid = db_register_packet->uid;
			client->tcpConnection.Send((char*)&sf_packet, sizeof(DB_SUCCESS_FAIL_PACKET));
		}
		break;
	}
	case E_PACKET::E_DB_LOGIN:	// 로그인	
	{
		DB_LOGIN_PACKET* db_login_packet = (DB_LOGIN_PACKET*)packet;
		cout << "DB_LOGIN_PACKET: " << db_login_packet->id << " " << db_login_packet->pw << endl;
		cout << "user: "<< db_login_packet->uid << endl;

		if (dbm.Login(db_login_packet->id, db_login_packet->pw)) {	// 로그인 성공
			DB_SUCCESS_FAIL_PACKET sf_packet;
			sf_packet.kind = static_cast<char>(E_PACKET::E_DB_LOGIN);
			sf_packet.result = 1;	// 성공
			sf_packet.uid = db_login_packet->uid;
			cout << "login success: " << db_login_packet->id << " " << db_login_packet->pw << endl;
			cout << "user: " << db_login_packet->uid << endl;
			cout<< "send packet: " << sf_packet.kind << " " << sf_packet.result << " " << sf_packet.uid << endl;
			client->tcpConnection.Send((char*)&sf_packet, sizeof(DB_SUCCESS_FAIL_PACKET));
		}
		else {
			DB_SUCCESS_FAIL_PACKET sf_packet;
			sf_packet.kind = static_cast<char>(E_PACKET::E_DB_LOGIN);
			sf_packet.result = 0;	// 실패
			sf_packet.uid = db_login_packet->uid;
			client->tcpConnection.Send((char*)&sf_packet, sizeof(DB_SUCCESS_FAIL_PACKET));
		}
		break;
	}
	default:
		break;
	}
}

void ProcessAccept()
{
	g_l_socket->m_isReadOverlapped = false;
	// 이미 accept은 완료되었다. 귀찮지만, Win32 AcceptEx 사용법에 따르는 마무리 작업을 하자. 
	if (remoteClientCandidate->tcpConnection.UpdateAcceptContext(*g_l_socket) != 0)
	{
		//리슨소켓을 닫았던지 하면 여기서 에러날거다. 그러면 리슨소켓 불능상태로 만들자.
		g_l_socket->Close();
	}
	else // 잘 처리함
	{
		shared_ptr<RemoteClient> remoteClient = remoteClientCandidate;
		cout << "accept - key: " << remoteClient.get() << endl;

		// 새 TCP 소켓도 IOCP에 추가한다.
		iocp.Add(remoteClient->tcpConnection, remoteClient.get());

		// overlapped 수신을 받을 수 있어야 하므로 여기서 I/O 수신 요청을 걸어둔다.
		if (remoteClient->tcpConnection.ReceiveOverlapped() != 0
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			// 에러. 소켓을 정리하자. 그리고 그냥 버리자.
			remoteClient->tcpConnection.Close();
		}
		else
		{
			// I/O를 걸었다. 완료를 대기하는 중 상태로 바꾸자.
			remoteClient->tcpConnection.m_isReadOverlapped = true;

			// 새 클라이언트를 목록에 추가.
			remoteClient->m_id = reinterpret_cast<unsigned long long>(remoteClient.get());
			RemoteClient::remoteClients.insert({ remoteClient.get(), remoteClient });
			cout << "Client joined. There are " << RemoteClient::remoteClients.size() << " connections.\n";

		}

		// 계속해서 소켓 받기를 해야 하므로 리슨소켓도 overlapped I/O를 걸자.
		remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);
		string errorText;
		if (!g_l_socket->AcceptOverlapped(remoteClientCandidate->tcpConnection, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			// 에러나면 리슨소켓 불능 상태로 남기자. 
			g_l_socket->Close();
		}
		else
		{
			// 리슨소켓은 연결이 들어옴을 기다리는 상태가 되었다.
			g_l_socket->m_isReadOverlapped = true;
		}
	}
}

