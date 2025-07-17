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
#include "Event.h"
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


#define IOCPCOUNT 5
const unsigned int iocpcount{ std::thread::hardware_concurrency() - 2 };

Iocp iocp(iocpcount); // �� ������ �����带 �� �ϳ��� ����. ���� ���⵵ 1�� ����.

recursive_mutex mx_accept;

shared_ptr<Socket> g_l_socket; // listensocket
shared_ptr<Socket> g_c_socket; // clientsocket
shared_ptr<PlayerClient>remoteClientCandidate;
vector<shared_ptr<thread>> worker_threads;

shared_ptr<thread> g_event_thread;
Timer g_timer;

std::atomic<bool> g_is_shutting_down{ false };

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
	if (dwCtrlType == CTRL_C_EVENT) {
		std::cout << "\nINFO: Ctrl+C ��ȣ ����. ���� ���� ������ �����մϴ�..." << std::endl;
		// ���� �÷��׸� true�� �����Ͽ� ��� ������ �ߴܽ�Ŵ
		g_is_shutting_down = true;

		// �ڵ鷯�� ��ȣ�� ó�������� �˸� (true�� ��ȯ�ؾ� ���� ���� ������ �����)
		return TRUE;
	}
	return FALSE;
}

void ProcessClientLeave(shared_ptr<PlayerClient> remoteClient)
{
	// ���� Ȥ�� ���� �����̴�.
	// �ش� ������ �����ع�����. 
	std::lock_guard<std::mutex> lock(remoteClient->c_mu);
	remoteClient->state = PC_FREE;
	Octree::PlayerOctree.remove(remoteClient->m_id);

	// �α׾ƿ� ���� ������
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
void BuildObject();

void worker_thread()
{
	try {
		while (!g_is_shutting_down)
		{
			// I/O �Ϸ� �̺�Ʈ�� ���� ������ ��ٸ��ϴ�.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// ���� �̺�Ʈ ������ ó���մϴ�.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				auto p_read_over = (OVER_EXP*)readEvent.lpOverlapped;

				if (readEvent.lpCompletionKey == 0 && p_read_over == nullptr) {
					// ������ ����
					return;
				}

				if (COMP_TYPE::OP_SEND == p_read_over->comp_type) {

					p_read_over->m_isReadOverlapped = false;
					delete p_read_over; // ���´ٸ� delete���ֱ�
					continue;
				}
				if (COMP_TYPE::OP_FSM_UPDATE == p_read_over->comp_type) // FSM ������Ʈ ��û�̸�
				{
					auto obj = GameObject::gameObjects[p_read_over->obj_id];
					if (obj) {
						obj->FSMUpdate();
					}
					delete p_read_over;
					continue;
				}
				if (readEvent.lpCompletionKey == (ULONG_PTR)g_l_socket.get()) // ���������̸�
				{
					ProcessAccept();				
				}
				else  // TCP ���� �����̸�
				{					
					// ó���� Ŭ���̾�Ʈ
					shared_ptr<PlayerClient> remoteClient;
					remoteClient = PlayerClient::PlayerClients[(PlayerClient*)readEvent.lpCompletionKey];
					if (remoteClient)
					{
						// �̹� ���ŵ� �����̴�. ���� �Ϸ�� ���� �׳� ���� ����.
						remoteClient->tcpConnection.m_isReadOverlapped = false;
						int recv_buf_length = readEvent.dwNumberOfBytesTransferred;

						if (recv_buf_length <= 0)
						{
							// ���� ����� 0 �� TCP ������ ������...
							// Ȥ�� ���� �� ���� ������ �� �����̴�...
							ProcessClientLeave(remoteClient);
						}
						else
						{
							// �̹� ���ŵ� �����̴�. ���� �Ϸ�� ���� �׳� ���� ����.
							char* recv_buf = remoteClient->tcpConnection.m_recv_over.send_buf;

							// ��Ŷ ������
							int remain_data = recv_buf_length + remoteClient->tcpConnection.m_prev_remain;
							while (remain_data > 0) {
								int packet_size = recv_buf[0];
								if (packet_size > remain_data) break;
								if (packet_size < 1) {
									remain_data = 0;
									break;
								}
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
void ProcessPacket(shared_ptr<PlayerClient>& client, char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_O_SETOBB:
	{
		OBJ_OBB_PACKET* r_packet = reinterpret_cast<OBJ_OBB_PACKET*>(packet);
		if (r_packet->oid < 0) { // �ش� Ŭ���� �÷��̾� obb
			if (OBB_Manager::obb_list.find(OBJECT_TYPE::OB_PLAYER) != OBB_Manager::obb_list.end()) {
				client->local_obb = *OBB_Manager::obb_list[OBJECT_TYPE::OB_PLAYER];
			}
			else {
				BoundingOrientedBox bb;
				bb.Center = XMFLOAT3{ r_packet->Center.x, r_packet->Center.y, r_packet->Center.z };
				bb.Extents = XMFLOAT3{ r_packet->Extents.x, r_packet->Extents.y, r_packet->Extents.z };
				bb.Orientation = XMFLOAT4{ r_packet->Orientation.x, r_packet->Orientation.y, r_packet->Orientation.z, r_packet->Orientation.w };
				OBB_Manager::AddOBB(OBJECT_TYPE::OB_PLAYER, bb);
				client->local_obb = *OBB_Manager::obb_list[OBJECT_TYPE::OB_PLAYER];
			}
		}
		else {
			OBJECT_TYPE type = GameObject::gameObjects[r_packet->oid]->GetType();
			if (OBB_Manager::obb_list.find(type) != OBB_Manager::obb_list.end()) break;
			BoundingOrientedBox bb;
			bb.Center = XMFLOAT3{ r_packet->Center.x, r_packet->Center.y, r_packet->Center.z };
			bb.Extents = XMFLOAT3{ r_packet->Extents.x, r_packet->Extents.y, r_packet->Extents.z };
			bb.Orientation = XMFLOAT4{ r_packet->Orientation.x, r_packet->Orientation.y, r_packet->Orientation.z, r_packet->Orientation.w };
			OBB_Manager::AddOBB(type, bb);
			for (auto& obj : GameObject::gameObjects) {
				if (obj->GetType() == type) {
					obj->local_obb = *OBB_Manager::obb_list[type];
					obj->UpdateTransform();
				}
			}	
		}
	}
	break;
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
		if (r_packet->inputData.Attack) {
			client->Playerstamina -= 5;
			CHANGE_STAT_PACKET s_packet;
			s_packet.size = sizeof(CHANGE_STAT_PACKET);
			s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
			s_packet.stat = E_STAT::STAMINA;
			s_packet.value = client->Playerstamina.load();
			client->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
		}
		client->BroadCastInputPacket();
	}
	break;
	case E_PACKET::E_O_HIT:
	{
		OBJ_HIT_PACKET* r_packet = reinterpret_cast<OBJ_HIT_PACKET*>(packet);
		if (GameObject::gameObjects.size() < r_packet->oid) return; // �߸��� id
		auto& obj = GameObject::gameObjects[r_packet->oid];
		obj->Decreasehp(r_packet->damage); // hp--
		if (obj->GetType() == OBJECT_TYPE::OB_PIG || obj->GetType() == OBJECT_TYPE::OB_COW) {
			if (obj->FSM_manager) {
				obj->SetInvincible();
				if (obj->Gethp() <= 0) obj->ChangeState(std::make_shared<NonAtkNPCDieState>());
				else obj->ChangeState(std::make_shared<NonAtkNPCRunAwayState>());
			}
		}
		else if (obj->GetType() == OBJECT_TYPE::OB_TREE || obj->GetType() == OBJECT_TYPE::OB_STONE) {
			
		}
		else {
			if (obj->FSM_manager) {
				obj->SetInvincible();
				if (obj->Gethp() <= 0) obj->ChangeState(std::make_shared<AtkNPCDieState>());
				else obj->ChangeState(std::make_shared<AtkNPCHitState>());
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
		if (GameObject::gameObjects.size() < r_packet->oid) return; // �߸��� id
		auto& obj = GameObject::gameObjects[r_packet->oid];
		obj->Sethp(r_packet->hp);
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			if (cl.second->m_id == client->m_id) continue;
			cl.second->SendHpPacket(obj->GetID(), obj->Gethp());
		}
	}
	break;
	case E_PACKET::E_P_SETHP:
	{
		SET_HP_HIT_OBJ_PACKET* r_packet = reinterpret_cast<SET_HP_HIT_OBJ_PACKET*>(packet);
		if(r_packet->hit_obj_id < 0) return; // �߸��� id
		auto o_type = GameObject::gameObjects[r_packet->hit_obj_id]->GetType();
		client->Playerhp -= GameObject::gameObjects[r_packet->hit_obj_id]->_atk;
		client->SetEffect(o_type);
	}
	break;
	case E_PACKET::E_P_CHANGE_STAT:
	{
		CHANGE_STAT_PACKET* r_packet = reinterpret_cast<CHANGE_STAT_PACKET*>(packet);
		client->Change_Stat(r_packet->stat, r_packet->value);
	}
	break;
	default:
		break;
	}
}

void event_thread()
{
	while (!g_is_shutting_down)
	{
		if (!EVENT::event_queue.empty()) {
			EVENT ev;
			if(EVENT::event_queue.try_pop(ev)) {
				if (ev.wakeup_time < std::chrono::system_clock::now())
				{
					switch (ev.e_type)
					{
					case EVENT_TYPE::E_P_SLOW_END: {
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							it.second->SetSlow(false);
							break;
						}
					}
					break;
					case EVENT_TYPE::E_P_REGENERATE_HP: {
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							int hp = it.second->Playerhp.load();

							while (hp < it.second->Maxhp.load()) {
								int desiredHp = hp + 1;
								if (desiredHp > it.second->Maxhp.load()) {
									desiredHp = it.second->Maxhp.load();
								}

								if (it.second->Playerhp.compare_exchange_weak(hp, desiredHp)) {
									// ��Ŷ����
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::HP;
									s_packet.value = it.second->Playerhp.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}

							EVENT::add_timer(ev, 1500); // 1.5�� �� �ٽ� hp ȸ��
							break;
						}
					}
						break;
					case EVENT_TYPE::E_P_REGENERATE_STAMINA: {
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							int stamina = it.second->Playerstamina.load();

							while (stamina < it.second->Maxstamina.load()) {
								int desiredstamina = stamina + 5;
								if (desiredstamina > it.second->Maxstamina.load()) {
									desiredstamina = it.second->Maxstamina.load();
								}

								if (it.second->Playerstamina.compare_exchange_weak(stamina, desiredstamina)) {
									// ��Ŷ����
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::STAMINA;
									s_packet.value = it.second->Playerstamina.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}
							EVENT::add_timer(ev, 1000); // 1�� �� �ٽ� stamina ȸ��
							break;
						}
					}
						break;
					case EVENT_TYPE::E_P_CONSUME_HUNGER: {
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							float expectedHunger = it.second->PlayerHunger.load();
							while (expectedHunger > 0.0f) {
								float desiredHunger = expectedHunger - 1.0f;

								if (desiredHunger < 0.0f) {
									desiredHunger = 0.0f;
								}

								if (it.second->PlayerHunger.compare_exchange_weak(expectedHunger, desiredHunger)) {
									// ��Ŷ����
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::HUNGER;
									s_packet.value = it.second->PlayerHunger.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}
							EVENT::add_timer(ev, 5000); // 5�ʸ��� ��� �Ҹ�						
							break;
						}
						
					}
						break;
					case EVENT_TYPE::E_P_CONSUME_THIRST: {
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							float expectedThirst = it.second->PlayerThirst.load();
							while (expectedThirst > 0.0f) {
								float desiredThirst;
								if (it.second->GetCurrentState() != ServerPlayerState::Idle)
									desiredThirst = expectedThirst - 2.0f;
								else
									desiredThirst = expectedThirst - 1.0f;
								if (desiredThirst < 0.0f) {
									desiredThirst = 0.0f;
								}
								if (it.second->PlayerThirst.compare_exchange_weak(expectedThirst, desiredThirst)) {
									// ��Ŷ����
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::THIRST;
									s_packet.value = it.second->PlayerThirst.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}
							EVENT::add_timer(ev, 2500); // 2.5�ʸ��� ���� �Ҹ�
							break;
						}
					}
						break;
					default:
						break;
					}

				}
				else EVENT::event_queue.push(ev);
			}
			else this_thread::sleep_for(chrono::milliseconds(1));
		}
		else this_thread::sleep_for(chrono::milliseconds(1));
	}
}

int main(int argc, char* argv[])
{
	SetConsoleTitle(L"GameServer");
	if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
		std::cerr << "����: �ܼ� ���� �ڵ鷯�� ����� �� �����ϴ�." << std::endl;
		return 1;
	}

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

		g_event_thread = make_shared<thread>(event_thread);

		g_timer.Start();
		while (!g_is_shutting_down) {
			g_timer.Tick(120.f);
			float deltaTime = g_timer.GetTimeElapsed(); // Use Tick same deltaTime

			for (auto& obj : GameObject::gameObjects) {
				std::vector<tree_obj*> results;
				tree_obj t_obj{ -1, obj->GetPosition() };
				Octree::PlayerOctree.query(t_obj, oct_distance, results);
				// Do not update if there is no player nearby
				if (results.size() > 0 && obj->FSM_manager) {
					OVER_EXP* p_over = new OVER_EXP();
					p_over->comp_type = COMP_TYPE::OP_FSM_UPDATE;
					p_over->obj_id = obj->GetID();
					PostQueuedCompletionStatus(iocp.m_hIocp, 0, (ULONG_PTR)obj.get(), &p_over->over);
				}
			}

			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME) continue;
				auto& beforepos = cl.second->GetPosition();
				if (cl.second->Playerstamina.load() > 0)
					cl.second->Update_test(deltaTime);
				auto& pos = cl.second->GetPosition();
				if (beforepos.x != pos.x || beforepos.y != pos.y || beforepos.z != pos.z)
				{
					cl.second->BroadCastPosPacket();
					int stamina = cl.second->Playerstamina.load();

					if (stamina > 0) {
						cl.second->stamina_counter++;
						if (cl.second->stamina_counter > 10) {
							int desiredstamina = stamina - 1;
							if (desiredstamina < 0) {
								desiredstamina = 0;
							}

							if (cl.second->Playerstamina.compare_exchange_weak(stamina, desiredstamina)) {
								// ��Ŷ����
								CHANGE_STAT_PACKET s_packet;
								s_packet.size = sizeof(CHANGE_STAT_PACKET);
								s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
								s_packet.stat = E_STAT::STAMINA;
								s_packet.value = cl.second->Playerstamina.load();
								cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
							}
							cl.second->stamina_counter = 0;
						}
					}
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
					if (0 == new_vl.count(o_id)) {	// before���� �ִٸ� ���� ��Ŷ
						cl.second->SendRemovePacket(GameObject::gameObjects[o_id]);
					}
				}
				for (auto o_id : new_vl) {
					if (0 == before_vl.count(o_id)) { //new���� �ִٸ� �߰� ��Ŷ
						if (GameObject::gameObjects[o_id]->is_alive == false) continue;
						if (GameObject::gameObjects[o_id]->Gethp() <= 0) continue;
						cl.second->SendAddPacket(GameObject::gameObjects[o_id]);
					}
				}
			}
		}
	}
	catch (Exception& e)
	{
		cout << "Exception! " << e.what() << endl;
		if (!g_is_shutting_down) {
			g_is_shutting_down = true; // �÷��� ���� �� ����
		}

	}
	CloseServer();
	return 0;
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


			LOGIN_PACKET s_packet;
			s_packet.size = sizeof(LOGIN_PACKET);
			s_packet.type = static_cast<char>(E_PACKET::E_P_LOGIN);
			s_packet.uid = remoteClient->m_id;
			// �� ���� ������
			remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));


			// ������ �������� �÷��̾� ���� ������
			for (auto& cl : PlayerClient::PlayerClients) {
				if(cl.second.get() == remoteClient.get()) continue; // �� �ڽ��� �����Ѵ�.
				LOGIN_PACKET s_a_packet;
				s_a_packet.size = sizeof(LOGIN_PACKET);
				s_a_packet.type = static_cast<char>(E_PACKET::E_P_LOGIN);
				s_a_packet.uid = cl.second->m_id;
				remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_a_packet));

				//// ��ġ�� ��������
				//POSITION_PACKET s_p_packet;
				//s_p_packet.size = sizeof(POSITION_PACKET);
				//s_p_packet.type = static_cast<unsigned char>(E_PACKET::E_P_POSITION);
				//s_p_packet.uid = cl.second->m_id;
				//auto& pos = cl.second->GetPosition();
				//s_p_packet.position.x = pos.x;
				//s_p_packet.position.y = pos.y;
				//s_p_packet.position.z = pos.z;
				//remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_p_packet));

				//// ȸ�� ������ ��������
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

			// ���� ���� ������
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second.get() == remoteClient.get()) continue; // �� �ڽ��� �����Ѵ�.
				cl.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
			}

		}

		// �ΰ��� ��ü �� ������ (���߿� ��Ʈ�� �̵� �� �丮��Ʈ ����)
		{
			std::vector<tree_obj*> results; // �þ� ���� �� ��ü ã��
			tree_obj p_obj{ -1,remoteClient->GetPosition() };
			Octree::GameObjectOctree.query(p_obj, oct_distance, results);
			for (auto& obj : results) {
				if (GameObject::gameObjects[obj->u_id]->is_alive == false) continue;
				if (GameObject::gameObjects[obj->u_id]->Gethp() <= 0) continue;
				remoteClient->SendAddPacket(GameObject::gameObjects[obj->u_id]);
			}				
		}
		auto p_obj = std::make_unique<tree_obj>(remoteClient->m_id, remoteClient->GetPosition());
		Octree::PlayerOctree.insert(std::move(p_obj));
		{
			std::lock_guard<std::mutex> lock(remoteClient->c_mu);
			remoteClient->state = PC_INGAME;
		}

		{
			EVENT ev(EVENT_TYPE::E_P_REGENERATE_HP, remoteClient->m_id, -1);
			EVENT::add_timer(ev, 1000); // 1�� �� hp ȸ��
			ev.e_type = EVENT_TYPE::E_P_REGENERATE_STAMINA;
			EVENT::add_timer(ev, 500); // 0.5�� �� stamina ȸ��
			ev.e_type = EVENT_TYPE::E_P_CONSUME_HUNGER;
			EVENT::add_timer(ev, 5000); // 5�ʸ��� ��� �Ҹ�
			ev.e_type = EVENT_TYPE::E_P_CONSUME_THIRST;
			EVENT::add_timer(ev, 2500); // 2.5�ʸ��� ���� �Ҹ�
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
void BuildObject()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	float spawnmin = 3000, spawnmax = 10000;
	float objectMinSize = 15, objectMaxSize = 20;

	int obj_id = 0;
	int TreeCount = 100;
	for (int i = 0; i < TreeCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();

		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		std::pair<float, float> randomsize = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		obj->SetScale(randomsize.first, randomsize.second, randomsize.first);
		obj->SetID(obj_id++);
		obj->SetType(OBJECT_TYPE::OB_TREE);
		obj->SetAnimationType(ANIMATION_TYPE::UNKNOWN);
		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int RockCount = 100;
	for (int i = 0; i < RockCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		std::pair<float, float> randomsize = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		obj->SetScale(randomsize.first, randomsize.second, randomsize.first);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_STONE);
		obj->SetAnimationType(ANIMATION_TYPE::UNKNOWN);
		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int CowCount = 70;
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

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int PigCount = 70;
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

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int SpiderCount = 50;
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

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int WolfCount = 50;
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

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int ToadCount = 50;
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

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int BatCount = 50;
	for (int i = 0; i < BatCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->fly_height = 15.f;
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second) + obj->fly_height, randompos.second);
		obj->SetScale(9.f, 9.f, 9.f);
		obj->SetID(obj_id++);
		obj->SetType(OBJECT_TYPE::OB_BAT);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int RaptorCount = 50;
	for (int i = 0; i < RaptorCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second) + 30, randompos.second);
		obj->SetScale(9.f, 9.f, 9.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_RAPTOR);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
}
void CloseServer()
{
	cout << "������ ��� ���ҽ��� �����ϰ� �����մϴ�." << endl;

	cout << "Waking up all worker threads..." << endl;
	for (int i = 0; i < iocpcount; ++i) {
		PostQueuedCompletionStatus(iocp.m_hIocp, 0, 0, NULL);
	}

	cout << "Waiting for worker threads to join..." << endl;
	for (auto& th : worker_threads) {
		if (th->joinable()) {
			th->join();
		}
	}
	cout << "All worker threads have been joined." << endl;

	cout << "Waiting for event thread to join..." << endl;
	if (g_event_thread->joinable()) {
		g_event_thread->join();
	}
	cout << "Event thread has been joined." << endl;

	cout << "Closing all sockets and clearing resources..." << endl;
	if (g_l_socket) {
		g_l_socket->Close();
	}
	for (auto& client_pair : PlayerClient::PlayerClients)
	{
		if (client_pair.second) {
			client_pair.second->tcpConnection.Close();
		}
	}
	PlayerClient::PlayerClients.clear(); // Ŭ���̾�Ʈ ��� ����

	cout << "������ ���������� ����Ǿ����ϴ�." << endl;
}