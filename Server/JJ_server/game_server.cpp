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
#include "GameObject.h"
#include "Octree.h"
#include <vector>


#include "NonAtkState.h"
#include "AtkState.h"

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
const int iocpcount{ 1 }; // CPU 코어 수 - 1

Iocp iocp(iocpcount); // 본 예제는 스레드를 딱 하나만 쓴다. 따라서 여기도 1이 들어간다.

recursive_mutex mx_accept;

shared_ptr<Socket> g_l_socket; // listensocket
shared_ptr<Socket> g_c_socket; // clientsocket
shared_ptr<PlayerClient>remoteClientCandidate;
vector<shared_ptr<thread>> worker_threads;


Timer g_timer;

std::vector<shared_ptr<GameObject>> gameObjects;

void ProcessClientLeave(shared_ptr<PlayerClient> remoteClient)
{
	// 에러 혹은 소켓 종료이다.
	// 해당 소켓은 제거해버리자. 
	std::lock_guard<std::mutex> lock(remoteClient->c_mu);
	remoteClient->state = PC_FREE;
	Octree::PlayerOctree.remove(remoteClient->m_id);

	// 로그아웃 정보 보내기
	for(auto& cl : PlayerClient::PlayerClients) {
		LOGOUT_PACKET s_packet;
		s_packet.size = sizeof(LOGOUT_PACKET);
		s_packet.type = static_cast<char>(E_PACKET::E_P_LOGOUT);
		s_packet.uid = remoteClient->m_id;
		cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
	}
	remoteClient->tcpConnection.Close();
	PlayerClient::PlayerClients.erase(remoteClient.get());

	cout << "Client left. There are " << PlayerClient::PlayerClients.size() << " connections.\n";
}
void ProcessPacket(shared_ptr<PlayerClient>& client, char* packet);
void ProcessAccept();
void CloseServer();
void worker_thread()
{
	try {
		while (1)
		{
			// I/O 완료 이벤트가 있을 때까지 기다립니다.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// 받은 이벤트 각각을 처리합니다.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				auto p_read_over = (OVER_EXP*)readEvent.lpOverlapped;

				if (COMP_TYPE::OP_SEND == p_read_over->comp_type) {

					p_read_over->m_isReadOverlapped = false;
					delete p_read_over; // 보냈다면 delete해주기
					continue;
				}

				if (readEvent.lpCompletionKey == (ULONG_PTR)g_l_socket.get()) // 리슨소켓이면
				{
					ProcessAccept();				
				}
				else  // TCP 연결 소켓이면
				{					
					//cout << "Recv!" << endl;
					// 처리할 클라이언트
					shared_ptr<PlayerClient> remoteClient;
					remoteClient = PlayerClient::PlayerClients[(PlayerClient*)readEvent.lpCompletionKey];
					if (remoteClient)
					{
						// 이미 수신된 상태이다. 수신 완료된 것을 그냥 꺼내 쓰자.
						remoteClient->tcpConnection.m_isReadOverlapped = false;
						int recv_buf_length = readEvent.dwNumberOfBytesTransferred;

						if (recv_buf_length <= 0)
						{
							// 읽은 결과가 0 즉 TCP 연결이 끝났다...
							// 혹은 음수 즉 뭔가 에러가 난 상태이다...
							ProcessClientLeave(remoteClient);
						}
						else
						{
							// 이미 수신된 상태이다. 수신 완료된 것을 그냥 꺼내 쓰자.
							char* recv_buf = remoteClient->tcpConnection.m_recv_over.send_buf;

							// 패킷 재조립
							int remain_data = recv_buf_length + remoteClient->tcpConnection.m_prev_remain;
							int packet_size = recv_buf[0];
							while (remain_data > 0 && packet_size <= remain_data) {
								ProcessPacket(remoteClient, recv_buf);
								recv_buf += packet_size;
								remain_data -= packet_size;
								packet_size = recv_buf[0];
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

void ProcessPacket(shared_ptr<PlayerClient>& client, char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_P_POSITION:
	{
		POSITION_PACKET* r_packet = reinterpret_cast<POSITION_PACKET*>(packet);
		client->SetPosition(XMFLOAT3{ r_packet->position.x, r_packet->position.y, r_packet->position.z });
		client->BroadCastPosPacket();
	}
	break;
	case E_PACKET::E_P_ROTATE:
	{
		ROTATE_PACKET* r_packet = reinterpret_cast<ROTATE_PACKET*>(packet);
		client->SetRight(XMFLOAT3{ r_packet->right.x,r_packet->right.y,r_packet->right.z });
		client->SetUp(XMFLOAT3{ r_packet->up.x,r_packet->up.y,r_packet->up.z });
		client->SetLook(XMFLOAT3{ r_packet->look.x,r_packet->look.y,r_packet->look.z });

		client->BroadCastRotatePacket();
	}
		break;
	case E_PACKET::E_P_INPUT:
	{
		INPUT_PACKET* r_packet = reinterpret_cast<INPUT_PACKET*>(packet);
		client->processInput(r_packet->inputData);
		client->BroadCastInputPacket();
	}
	break;
	case E_PACKET::E_O_HIT:
	{
		OBJ_HIT_PACKET* r_packet = reinterpret_cast<OBJ_HIT_PACKET*>(packet);
		if (gameObjects.size() < r_packet->oid) return; // 잘못된 id
		auto& obj = gameObjects[r_packet->oid];
		obj->Decreasehp(r_packet->damage); // hp--
		std::cout << "Recv hit packet - damage: " << r_packet->damage << " hp: " << obj->Gethp() << std::endl;
		if (obj->GetType() == OBJECT_TYPE::OB_PIG || obj->GetType() == OBJECT_TYPE::OB_COW) {
			if (obj->FSM_manager) {
				obj->FSM_manager->SetInvincible();
				if (obj->Gethp() <= 0) obj->FSM_manager->ChangeState(std::make_shared<NonAtkNPCDieState>());
				else obj->FSM_manager->ChangeState(std::make_shared<NonAtkNPCRunAwayState>());
			}
		}
		else if (obj->GetType() == OBJECT_TYPE::OB_TREE || obj->GetType() == OBJECT_TYPE::OB_STONE) {
			
		}
		else {
			if (obj->FSM_manager) {
				obj->FSM_manager->SetInvincible();
				if (obj->Gethp() <= 0) obj->FSM_manager->ChangeState(std::make_shared<AtkNPCDieState>());
				else obj->FSM_manager->ChangeState(std::make_shared<AtkNPCHitState>());
			}
		}
		for(auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			cl.second->SendHpPacket(r_packet->oid, obj->Gethp());
			cl.second->SendInvinciblePacket(r_packet->oid, true);
		}		
	}
		break;

	case E_PACKET::E_O_SETHP:
	{
		OBJ_HP_PACKET* r_packet = reinterpret_cast<OBJ_HP_PACKET*>(packet);
		if (gameObjects.size() < r_packet->oid) return; // 잘못된 id
		auto& obj = gameObjects[r_packet->oid];
		obj->Sethp(r_packet->hp);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			if (cl.second->m_id == client->m_id) continue;
			cl.second->SendHpPacket(obj->GetID(), obj->Gethp());
		}
	}
	break;

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
		shared_ptr<PlayerClient> remoteClient = remoteClientCandidate;
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
			PlayerClient::PlayerClients.insert({ remoteClient.get(), remoteClient });
			cout << "Client joined. There are " << PlayerClient::PlayerClients.size() << " connections.\n";
			cout <<" Client id: "<< remoteClient->m_id << endl;


			LOGIN_PACKET s_packet;
			s_packet.size = sizeof(LOGIN_PACKET);
			s_packet.type = static_cast<char>(E_PACKET::E_P_LOGIN);
			s_packet.uid = remoteClient->m_id;
			// 내 정보 보내기
			remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));


			// 나에게 접속중인 플레이어 정보 보내기
			for (auto& cl : PlayerClient::PlayerClients) {
				if(cl.second.get() == remoteClient.get()) continue; // 나 자신은 제외한다.
				LOGIN_PACKET s_a_packet;
				s_a_packet.size = sizeof(LOGIN_PACKET);
				s_a_packet.type = static_cast<char>(E_PACKET::E_P_LOGIN);
				s_a_packet.uid = cl.second->m_id;
				remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_a_packet));

				//// 위치도 전송하자
				//POSITION_PACKET s_p_packet;
				//s_p_packet.size = sizeof(POSITION_PACKET);
				//s_p_packet.type = static_cast<unsigned char>(E_PACKET::E_P_POSITION);
				//s_p_packet.uid = cl.second->m_id;
				//auto& pos = cl.second->GetPosition();
				//s_p_packet.position.x = pos.x;
				//s_p_packet.position.y = pos.y;
				//s_p_packet.position.z = pos.z;
				//remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_p_packet));

				//// 회전 정보도 전송하자
				//ROTATE_PACKET s_r_packet;
				//s_r_packet.size = sizeof(ROTATE_PACKET);
				//s_r_packet.type = static_cast<unsigned char>(E_PACKET::E_P_ROTATE);
				//s_r_packet.uid = cl.second->m_id;
				//auto& right = cl.second->GetRight();
				//auto& up = cl.second->GetUp();
				//auto& look = cl.second->GetLook();
				//s_r_packet.right.x = right.x;
				//s_r_packet.right.y = right.y;
				//s_r_packet.right.z = right.z;
				//s_r_packet.up.x = up.x;
				//s_r_packet.up.y = up.y;
				//s_r_packet.up.z = up.z;
				//s_r_packet.look.x = look.x;
				//s_r_packet.look.y = look.y;
				//s_r_packet.look.z = look.z;
				//remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_r_packet));
			}

			// 나의 정보 보내기
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second.get() == remoteClient.get()) continue; // 나 자신은 제외한다.
				cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
			}

		}

		// 인게임 객체 다 보내기 (나중에 옥트리 이동 시 뷰리스트 적용)
		{
			std::vector<tree_obj*> results; // 시야 범위 내 객체 찾기
			tree_obj p_obj{ -1,remoteClient->GetPosition() };
			Octree::GameObjectOctree.query(p_obj, oct_distance, results);
			for (auto& obj : results) {
				if (gameObjects[obj->u_id]->is_alive == false) continue;
				if (gameObjects[obj->u_id]->Gethp() <= 0) continue;
				remoteClient->SendAddPacket(gameObjects[obj->u_id]);
			}				
		}
		auto p_obj = std::make_unique<tree_obj>(remoteClient->m_id, remoteClient->GetPosition());
		Octree::PlayerOctree.insert(std::move(p_obj));
		{
			std::lock_guard<std::mutex> lock(remoteClient->c_mu);
			remoteClient->state = PC_INGAME;
		}

		// 계속해서 소켓 받기를 해야 하므로 리슨소켓도 overlapped I/O를 걸자.
		remoteClientCandidate = make_shared<PlayerClient>(SocketType::Tcp);
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


void BuildObject()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	float spawnmin = 1000, spawnmax = 10000;
	float objectMinSize = 15, objectMaxSize = 20;

	int obj_id = 0;
	int TreeCount = 500;
	for (int i = 0; i < TreeCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();

		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		std::pair<float, float> randomsize = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		obj->SetScale(randomsize.first, randomsize.second, randomsize.first);
		obj->SetID(obj_id++);
		obj->SetType(OBJECT_TYPE::OB_TREE);
		obj->SetAnimationType(ANIMATION_TYPE::UNKNOWN);
		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int RockCount = 400;
	for (int i = 0; i < RockCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		std::pair<float, float> randomsize = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		obj->SetScale(randomsize.first, randomsize.second, randomsize.first);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_STONE);
		obj->SetAnimationType(ANIMATION_TYPE::UNKNOWN);
		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int CowCount = 150;
	for (int i = 0; i < CowCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(12.f, 12.f, 12.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_COW);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int PigCount = 150;
	for (int i = 0; i < PigCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(10.f, 10.f, 10.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_PIG);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int SpiderCount = 150;
	for (int i = 0; i < SpiderCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(8.f, 8.f, 8.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_SPIDER);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int WolfCount = 150;
	for (int i = 0; i < WolfCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(10.f, 10.f, 10.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_WOLF);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int ToadCount = 150;
	for (int i = 0; i < ToadCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(8.f, 8.f, 8.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_TOAD);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
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


		BuildObject();
		std::cout<<"BuildObject complete!"<< std::endl;

		for (int i{}; i < iocpcount; ++i)
			worker_threads.emplace_back(make_shared<thread>(worker_thread));


		g_timer.Start();
		while (true) {
			g_timer.Tick(60.f);
			float deltaTime = g_timer.GetTimeElapsed(); // Use Tick same deltaTime

			// fsm몬스터 로직
			for (auto& obj : gameObjects) {
				std::vector<tree_obj*> results;
				tree_obj t_obj{ -1, obj->GetPosition() };
				Octree::PlayerOctree.query(t_obj, oct_distance, results);
				// Do not update if there is no player nearby
				if (results.size() > 0 && obj->FSM_manager) obj->FSMUpdate();
			}

			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME) continue;
				auto& beforepos = cl.second->GetPosition();
				cl.second->Update_test(deltaTime);
				auto& pos = cl.second->GetPosition();
				if (beforepos.x != pos.x || beforepos.y != pos.y || beforepos.z != pos.z)
				{
					cl.second->BroadCastPosPacket();
				}
				Octree::PlayerOctree.update(cl.second->m_id, cl.second->GetPosition());


				cl.second->vl_mu.lock();
				std::unordered_set<int> before_vl = cl.second->viewlist;
				cl.second->vl_mu.unlock();

				std::unordered_set<int> new_vl;
				std::vector<tree_obj*> results; // find obj
				tree_obj p_obj{ -1,pos };
				Octree::GameObjectOctree.query(p_obj, oct_distance, results);
				for (auto& obj : results) new_vl.insert(obj->u_id);

				for (auto o_id : before_vl) {
					if (0 == new_vl.count(o_id)) {	// before에만 있다면 제거 패킷
						cl.second->SendRemovePacket(gameObjects[o_id]);
					}
				}
				for (auto o_id : new_vl) {
					if (0 == before_vl.count(o_id)) { //new에만 있다면 추가 패킷
						if (gameObjects[o_id]->is_alive == false) continue;
						if (gameObjects[o_id]->Gethp() <= 0) continue;
						cl.second->SendAddPacket(gameObjects[o_id]);
					}
				}
			}
		}

		for (auto& th : worker_threads) th->join();
	}
	catch (Exception& e)
	{
		cout << "Exception! " << e.what() << endl;

	}
	CloseServer();
	return 0;
}

void CloseServer()
{
	lock_guard<recursive_mutex> lock_accept(mx_accept);
	// i/o 완료 체크
	g_l_socket->Close();


	for (auto i : PlayerClient::PlayerClients)
	{
		i.second->tcpConnection.Close();
	}


	// 서버를 종료하기 위한 정리중
	cout << "서버를 종료하고 있습니다...\n";
	while (PlayerClient::PlayerClients.size() > 0)
	{
		// I/O completion이 없는 상태의 RemoteClient를 제거한다.
		for (auto i = PlayerClient::PlayerClients.begin(); i != PlayerClient::PlayerClients.end(); ++i)
		{
			if (!i->second->tcpConnection.m_isReadOverlapped) {
				PlayerClient::PlayerClients.erase(i);
			}
		}

		// I/O completion이 발생하면 더 이상 Overlapped I/O를 걸지 말고 정리해야함을 나타낸다.
		IocpEvents readEvents;
		iocp.Wait(readEvents, 100);

		// 받은 이벤트 각각을 처리합니다.
		for (int i = 0; i < readEvents.m_eventCount; i++)
		{
			auto& readEvent = readEvents.m_events[i];
			if (readEvent.lpCompletionKey == 0) // 리슨소켓이면
			{
				g_l_socket->m_isReadOverlapped = false;
			}
			else
			{
				shared_ptr<PlayerClient> remoteClient = PlayerClient::PlayerClients[(PlayerClient*)readEvent.lpCompletionKey];
				if (remoteClient)
				{
					remoteClient->tcpConnection.m_isReadOverlapped = false;
				}
			}
		}
	}

	cout << "서버 끝.\n";
}