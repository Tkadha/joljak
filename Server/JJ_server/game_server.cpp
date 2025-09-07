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

#include "BossState.h"
#include "NonAtkState.h"
#include "AtkState.h"

using namespace std;

#define LOBBY_PORT 8999
#define PORT 9000
#define DB_PORT 9001

#define MIN_HEIGHT                  1055.f      
//-----------------------------------------------
// 
//				game server
// 
//-----------------------------------------------


#define IOCPCOUNT 5
const unsigned int iocpcount{ std::thread::hardware_concurrency() - 2 };

Iocp iocp(iocpcount); // 본 예제는 스레드를 딱 하나만 쓴다. 따라서 여기도 1이 들어간다.

recursive_mutex mx_accept;

shared_ptr<Socket> g_l_socket; // listensocket
shared_ptr<Socket> g_c_socket; // clientsocket
shared_ptr<PlayerClient>remoteClientCandidate;
vector<shared_ptr<thread>> worker_threads;

shared_ptr<thread> g_event_thread;
Timer g_timer;

std::atomic<bool> g_is_shutting_down{ false };
std::atomic<bool> g_is_start_game{ false };

static float time_accumulator = 0.f;
static int play_day = 0;

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
	if (dwCtrlType == CTRL_C_EVENT) {
		std::cout << "\nINFO: Ctrl+C 신호 감지. 서버 종료 절차를 시작합니다..." << std::endl;
		// 전역 플래그를 true로 설정하여 모든 루프를 중단시킴
		g_is_shutting_down = true;

		// 핸들러가 신호를 처리했음을 알림 (true를 반환해야 정상 종료 로직이 실행됨)
		return TRUE;
	}
	return FALSE;
}

bool InHeilpad(float x, float z)
{
	return x >= 7600 && x <= 8700 && z >= 7600 && z <= 8700;
}

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
	std::lock_guard<std::mutex> g_lock(g_clients_mutex);
	PlayerClient::PlayerClients.erase(remoteClient.get());

	cout << "Client left. There are " << PlayerClient::PlayerClients.size() << " connections.\n";
}
void ProcessPacket(shared_ptr<PlayerClient>& client, char* packet);
void ProcessAccept();
void CloseServer();
void BuildObject();
void ReleaseObject();
void worker_thread()
{
	try {
		while (!g_is_shutting_down)
		{
			// I/O 완료 이벤트가 있을 때까지 기다립니다.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// 받은 이벤트 각각을 처리합니다.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				auto p_read_over = (OVER_EXP*)readEvent.lpOverlapped;

				if (readEvent.lpCompletionKey == 0 && p_read_over == nullptr) {
					// 스레드 종료
					return;
				}

				if (readEvent.lpCompletionKey == (ULONG_PTR)g_l_socket.get()) // 리슨소켓이면
				{
					ProcessAccept();				
				}
				else if (COMP_TYPE::OP_SEND == p_read_over->comp_type) {

					p_read_over->m_isReadOverlapped = false;
					delete p_read_over; // 보냈다면 delete해주기
				}
				else if (COMP_TYPE::OP_FSM_UPDATE == p_read_over->comp_type) // FSM 업데이트 요청이면
				{
					if (g_is_start_game) {
						auto obj = GameObject::gameObjects[p_read_over->obj_id];
						if (obj && obj->IsRenderObj()) {
							obj->FSMUpdate();
						}
					}
					delete p_read_over;
				}
				else if (COMP_TYPE::OP_PLAYER_UPDATE == p_read_over->comp_type) {
					if (!g_is_start_game) {
						delete p_read_over;
						continue;
					}
					shared_ptr<PlayerClient> Client;
					auto it = PlayerClient::PlayerClients.find((PlayerClient*)readEvent.lpCompletionKey);
					if (it != PlayerClient::PlayerClients.end()) {
						Client = it->second;
					}
					else {
						delete p_read_over;
						continue;
					}
					std::lock_guard<std::mutex> lock(Client->c_mu);
					if (Client->state != PC_INGAME) continue;
					float deltaTime = g_timer.GetTimeElapsed();
					auto& beforepos = Client->GetPosition();
					if (Client->Playerstamina.load() > 0)
						Client->Update_test(deltaTime);
					auto& pos = Client->GetPosition();
					if (beforepos.x != pos.x || beforepos.y != pos.y || beforepos.z != pos.z)
					{
						Client->BroadCastPosPacket();
						int stamina = Client->Playerstamina.load();
						if (stamina > 0 && Client->GetCurrentState() == ServerPlayerState::Running) {
							Client->stamina_counter++;
							if (Client->stamina_counter > 20) {
								int desiredstamina = stamina - 1;
								if (desiredstamina < 0) {
									desiredstamina = 0;
								}
								if (Client->Playerstamina.compare_exchange_weak(stamina, desiredstamina)) {
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::STAMINA;
									s_packet.value = Client->Playerstamina.load();
									Client->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
								}
								Client->stamina_counter = 0;
							}
						}
					}
					Octree::PlayerOctree.update(Client->m_id, Client->GetPosition());
					Client->UpdateTransform();

					Client->vl_mu.lock();
					std::unordered_set<int> before_vl = Client->viewlist;
					Client->vl_mu.unlock();

					std::unordered_set<int> new_vl;
					std::vector<tree_obj*> results; // find obj
					tree_obj p_obj{ -1,pos };
					Octree::GameObjectOctree.query(p_obj, oct_distance, results);
					for (auto& obj : results) new_vl.insert(obj->u_id);

					for (auto o_id : before_vl) {
						if (0 == new_vl.count(o_id)) {	// before에만 있다면 제거 패킷
							Client->SendRemovePacket(GameObject::gameObjects[o_id]);
						}
						else if (1 == new_vl.count(o_id)) {
							if (GameObject::gameObjects[o_id]->is_alive == true) continue;
							if (GameObject::gameObjects[o_id]->Gethp() > 0) continue;
							Client->SendRemovePacket(GameObject::gameObjects[o_id]);
						}
					}
					for (auto o_id : new_vl) {
						if (0 == before_vl.count(o_id)) { //new에만 있다면 추가 패킷
							if (GameObject::gameObjects[o_id]->is_alive == false) continue;
							if (GameObject::gameObjects[o_id]->Gethp() <= 0) continue;
							Client->SendAddPacket(GameObject::gameObjects[o_id]);
						}
					}
					Client->vl_mu.lock();
					Client->viewlist = new_vl;
					Client->vl_mu.unlock();

					delete p_read_over;
				}
				else  // TCP 연결 소켓이면
				{					
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
	case E_PACKET::E_CHANGE_TIME:
	{
		XMVECTOR xmvBaseLightDirection = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		XMMATRIX xmmtxLightRotate = XMMatrixRotationZ(XMConvertToRadians(time_accumulator));
		XMVECTOR xmvCurrentLightDirection = XMVector3TransformNormal(xmvBaseLightDirection, xmmtxLightRotate);

		if (XMVectorGetY(xmvCurrentLightDirection) < 0.0f) // 빛이 아래를 향하면 낮
		{
			time_accumulator = 180.f;
		}
		else // 빛이 위를 향하면 밤
		{
			time_accumulator = 0.f;
		}
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			cl.second->SendTimePacket(time_accumulator);
		}
	}
		break;
	case E_PACKET::E_P_WEAPON_CHANGE: 
	{
		WEAPON_CHANGE_PACKET* r_packet = reinterpret_cast<WEAPON_CHANGE_PACKET*>(packet);
		client->BroadCastWeaponPacket(*r_packet);
	}
	break;
	case E_PACKET::E_O_SETOBB:
	{
		OBJ_OBB_PACKET* r_packet = reinterpret_cast<OBJ_OBB_PACKET*>(packet);
		if (r_packet->oid < 0) { // 해당 클라의 플레이어 obb
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
			if (client->Playerstamina.load() < 0) client->Playerstamina.store(0);
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
		if (GameObject::gameObjects.size() < r_packet->oid) return; // 잘못된 id
		auto& obj = GameObject::gameObjects[r_packet->oid];
		obj->Decreasehp(r_packet->damage); // hp--
		if (obj->GetType() == OBJECT_TYPE::OB_PIG || obj->GetType() == OBJECT_TYPE::OB_COW) {
			if (obj->FSM_manager) {
				obj->SetInvincible();
				if (obj->Gethp() <= 0) obj->ChangeState(std::make_shared<NonAtkNPCDieState>());
				else obj->ChangeState(std::make_shared<NonAtkNPCRunAwayState>());
			}
		}
		else if (obj->GetType() == OBJECT_TYPE::OB_GOLEM) {
			if (obj->FSM_manager) {
				auto bosshp = obj->Gethp();
				if (bosshp <= 0) {
					obj->SetInvincible();
					obj->ChangeState(std::make_shared<BossDieState>());
				}
				else if (bosshp <= (obj->_fMaxHp / 2) && !obj->_bUsedSpecialAttack) {
					obj->SetInvincible(5.332f * 1000);
					obj->_bUsedSpecialAttack = true; // 플래그를 true로 설정
					obj->ChangeState(std::make_shared<BossSpecialAttackStartState>()); // 별도의 특수 공격 상태
				}
				else if (bosshp <= obj->_fMaxHp * 0.33f && !obj->_bTriggered33Percent) {
					obj->SetInvincible();
					obj->_bTriggered33Percent = true; // 플래그를 true로 설정해 다시는 실행되지 않도록 함
					obj->ChangeState(std::make_shared<BossHitState>());
				}
				// 3. 체력 66% 이하로 '처음' 떨어졌을 때
				else if (bosshp <= obj->_fMaxHp * 0.66f && !obj->_bTriggered66Percent) {
					obj->SetInvincible();
					obj->_bTriggered66Percent = true; // 플래그를 true로 설정
					obj->ChangeState(std::make_shared<BossHitState>());
				}
				else
					obj->SetInvincible();
			}
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
		if (GameObject::gameObjects.size() < r_packet->oid) return; // 잘못된 id
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
		if(r_packet->hit_obj_id < 0) return; // 잘못된 id
		auto o_type = GameObject::gameObjects[r_packet->hit_obj_id]->GetType();
		client->Playerhp -= GameObject::gameObjects[r_packet->hit_obj_id]->_atk;
		if (client->Playerhp.load() < 0) {
			client->RespawnPlayer();
			PLAYER_RESPAWN_PACKET p;
			p.size = sizeof(PLAYER_RESPAWN_PACKET);
			p.type = static_cast<char>(E_PACKET::E_P_RESPAWN);
			client->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&p));
			client->BroadCastPosPacket();
		}
		CHANGE_STAT_PACKET s_packet;
		s_packet.size = sizeof(CHANGE_STAT_PACKET);
		s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
		s_packet.stat = E_STAT::HP;
		s_packet.value = client->Playerhp.load();
		client->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));

		// 피격 애니메이션 전송
		PlayerInput pi;
		pi.Hit = true;
		client->BroadCastHitPacket(pi);
		client->SetEffect(o_type);
	}
	break;
	case E_PACKET::E_P_CHANGE_STAT:
	{
		CHANGE_STAT_PACKET* r_packet = reinterpret_cast<CHANGE_STAT_PACKET*>(packet);
		client->Change_Stat(r_packet->stat, r_packet->value);
	}
	break;
	case E_PACKET::E_STRUCT_OBJ:
	{
		STRUCT_OBJ_PACKET* r_packet = reinterpret_cast<STRUCT_OBJ_PACKET*>(packet);
		std::shared_ptr<GameObject> obj = std::make_shared<GameObject>();
		BoundingOrientedBox bb;
		bb.Center = XMFLOAT3{ r_packet->Center.x, r_packet->Center.y, r_packet->Center.z };
		bb.Extents = XMFLOAT3{ r_packet->Extents.x, r_packet->Extents.y, r_packet->Extents.z };
		bb.Orientation = XMFLOAT4{ r_packet->Orientation.x, r_packet->Orientation.y, r_packet->Orientation.z, r_packet->Orientation.w };
		obj->local_obb = bb;
		obj->SetRight(XMFLOAT3{ r_packet->right.x, r_packet->right.y, r_packet->right.z });
		obj->SetUp(XMFLOAT3{ r_packet->up.x, r_packet->up.y, r_packet->up.z });
		obj->SetLook(XMFLOAT3{ r_packet->look.x, r_packet->look.y, r_packet->look.z });
		obj->SetPosition(XMFLOAT3{ r_packet->position.x, r_packet->position.y, r_packet->position.z });
		obj->SetType(r_packet->o_type);
		GameObject::ConstructObjects.push_back(obj);

		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			if (cl.second->m_id == client->m_id) continue;
			cl.second->SendStructPacket(obj);
		}
	}
	break;
	case E_PACKET::E_GAME_START:
	{
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			cl.second->SendStartGamePacket();
			cl.second->SendTimePacket(time_accumulator);
		}
		std::cout << "start game" << std::endl;
		g_is_start_game.store(true);
	}
	break;
	case E_PACKET::E_GAME_END:
	{
		if (g_is_start_game.load()) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME) continue;
				cl.second->SendEndGamePacket();
			}
			std::cout << "end game" << std::endl;
			g_is_start_game.store(false);
			EVENT ev{ EVENT_TYPE::E_REBUILD_OBJ,0,-1 };
			EVENT::add_timer(ev, 1000);
		}
	}
	break;
	case E_PACKET::E_GAME_NEW:
	{
		if (!g_is_start_game.load()) {	

			for (auto& cl : PlayerClient::PlayerClients)
			{
				auto& player = cl.second;
				player->SendNewGamePacket();
			}
			for (auto& cl : PlayerClient::PlayerClients)
			{
				auto& player = cl.second;
				cl.second->ResetState();
				std::vector<tree_obj*> results; // 시야 범위 내 객체 찾기
				tree_obj p_obj{ -1,player->GetPosition() };
				Octree::GameObjectOctree.query(p_obj, oct_distance, results);
				std::unordered_set<int> new_vl;
				for (auto& obj : results) new_vl.insert(obj->u_id);
				for (auto& obj : results) {
					if (GameObject::gameObjects[obj->u_id]->is_alive == false) continue;
					if (GameObject::gameObjects[obj->u_id]->Gethp() <= 0) continue;
					player->SendAddPacket(GameObject::gameObjects[obj->u_id]);
				}
				for (auto& obj : GameObject::ConstructObjects) {
					player->SendStructPacket(obj);
				}
				player->vl_mu.lock();
				player->viewlist = new_vl;
				player->vl_mu.unlock();

				player->SendTimePacket(time_accumulator);

				player->BroadCastPosPacket();
			}
		}
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
						if (!g_is_start_game.load()) {
							EVENT::add_timer(ev, 1500);
							break;
						}
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
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::HP;
									s_packet.value = it.second->Playerhp.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}

							EVENT::add_timer(ev, 1500); // 1.5초 후 다시 hp 회복
							break;
						}
					}
						break;
					case EVENT_TYPE::E_P_REGENERATE_STAMINA: {
						if (!g_is_start_game.load()) {
							EVENT::add_timer(ev, 750);
							break;
						}
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							int stamina = it.second->Playerstamina.load();

							while (stamina < it.second->Maxstamina.load()) {
								int desiredstamina = stamina + 2;
								if (desiredstamina > it.second->Maxstamina.load()) {
									desiredstamina = it.second->Maxstamina.load();
								}

								if (it.second->Playerstamina.compare_exchange_weak(stamina, desiredstamina)) {
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::STAMINA;
									s_packet.value = it.second->Playerstamina.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}
							EVENT::add_timer(ev, 750); // 0.75초 후 다시 stamina 회복
							break;
						}
					}
						break;
					case EVENT_TYPE::E_P_CONSUME_HUNGER: {
						if (!g_is_start_game.load()) {
							EVENT::add_timer(ev, 5000);
							break;
						}
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
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::HUNGER;
									s_packet.value = it.second->PlayerHunger.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}
							EVENT::add_timer(ev, 5000); // 5초마다 허기 소모						
							break;
						}
						
					}
						break;
					case EVENT_TYPE::E_P_CONSUME_THIRST: {
						if (!g_is_start_game.load()) {
							EVENT::add_timer(ev, 4000);
							break;
						}
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							float expectedThirst = it.second->PlayerThirst.load();
							while (expectedThirst > 0.0f) {
								float desiredThirst;
								if (it.second->GetCurrentState() == ServerPlayerState::Idle || it.second->GetCurrentState() == ServerPlayerState::Walking)
									desiredThirst = expectedThirst - 1.0f;
								else
									desiredThirst = expectedThirst - 2.0f;
								if (desiredThirst < 0.0f) {
									desiredThirst = 0.0f;
								}
								if (it.second->PlayerThirst.compare_exchange_weak(expectedThirst, desiredThirst)) {
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::THIRST;
									s_packet.value = it.second->PlayerThirst.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}
							EVENT::add_timer(ev, 4000); // 4초마다 갈증 소모
							break;
						}
					}
						break;
					case EVENT_TYPE::E_P_BLEEDING: {
						if (!g_is_start_game.load()) break;
						if (ev.end_time < std::chrono::system_clock::now()) break;
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							int hp = it.second->Playerhp.load();

							while (hp > 0) {
								int desiredHp = hp - 2;
								if (desiredHp < 0) {
									desiredHp = 0;
								}

								if (it.second->Playerhp.compare_exchange_weak(hp, desiredHp)) {
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::HP;
									s_packet.value = it.second->Playerhp.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}

							EVENT::add_timer(ev, 1000); // 1.0초 후 다시 출혈
							break;
						}
					}
						break;
					case EVENT_TYPE::E_P_POISON: {
						if (!g_is_start_game.load()) break;						
						if (ev.end_time < std::chrono::system_clock::now()) break;
						auto uid = ev.player_id;
						for (auto it : PlayerClient::PlayerClients) {
							if (it.second->m_id != uid) continue;
							int stamina = it.second->Playerstamina.load();

							while (stamina > 0) {
								int desiredstamina = stamina - 1;
								if (desiredstamina < 0) {
									desiredstamina = 0;
								}

								if (it.second->Playerstamina.compare_exchange_weak(stamina, desiredstamina)) {
									// 패킷전송
									CHANGE_STAT_PACKET s_packet;
									s_packet.size = sizeof(CHANGE_STAT_PACKET);
									s_packet.type = static_cast<char>(E_PACKET::E_P_CHANGE_STAT);
									s_packet.stat = E_STAT::STAMINA;
									s_packet.value = it.second->Playerstamina.load();
									it.second->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));
									break;
								}
							}

							EVENT::add_timer(ev, 500); // 1.0초 후 다시 출혈
							break;
						}
					}
						break;
					case EVENT_TYPE::E_END_GAME: {
						for (auto& cl : PlayerClient::PlayerClients) {
							cl.second->SendEndGamePacket();
						}
						g_is_start_game.store(false);
					}
						break;
					case EVENT_TYPE::E_REBUILD_OBJ:
					{
						g_is_start_game.store(false);
						ReleaseObject();
						std::cout << "Release all obj" << std::endl;
						BuildObject();
						time_accumulator = 0;
						std::cout << "Build obj" << std::endl;
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
		std::cerr << "오류: 콘솔 제어 핸들러를 등록할 수 없습니다." << std::endl;
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
			if (!g_is_start_game) continue;
			float deltaTime = g_timer.GetTimeElapsed(); // Use Tick same deltaTime
			float time_speed = 0.1f;
			time_accumulator += deltaTime * time_speed;
			if (time_accumulator > 360.0f) {
				time_accumulator -= 360.0f;
				play_day++;
				for (auto& cl : PlayerClient::PlayerClients) {
					if (cl.second->state != PC_INGAME) continue;
					cl.second->SendTimePacket(time_accumulator);
				}
			}
			XMVECTOR xmvBaseLightDirection = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
			XMMATRIX xmmtxLightRotate = XMMatrixRotationZ(XMConvertToRadians(time_accumulator));
			XMVECTOR xmvCurrentLightDirection = XMVector3TransformNormal(xmvBaseLightDirection, xmmtxLightRotate);

			if (XMVectorGetY(xmvCurrentLightDirection) < 0.0f) // 빛이 아래를 향하면 낮
			{
				if (g_is_night.load()) {
					g_is_night.store(false);
				}
			}
			else // 빛이 위를 향하면 밤
			{
				if (!g_is_night.load()) {
					g_is_night.store(true);
				}
			}
			for (auto& obj : GameObject::gameObjects) {
				std::vector<tree_obj*> results;
				tree_obj t_obj{ -1, obj->GetPosition() };
				Octree::PlayerOctree.query(t_obj, oct_distance, results);
				if (results.size() > 0 && obj->FSM_manager) {
					OVER_EXP* p_over = new OVER_EXP();
					p_over->comp_type = COMP_TYPE::OP_FSM_UPDATE;
					p_over->obj_id = obj->GetID();
					PostQueuedCompletionStatus(iocp.m_hIocp, 0, (ULONG_PTR)obj.get(), &p_over->over);
				}
			}
			g_clients_mutex.lock();
			for (auto& cl : PlayerClient::PlayerClients) {
				{
					auto& client = cl.second;
					if (client->state != PC_INGAME) continue;
					OVER_EXP* p_over = new OVER_EXP();
					p_over->comp_type = COMP_TYPE::OP_PLAYER_UPDATE;
					PostQueuedCompletionStatus(iocp.m_hIocp, 0, (ULONG_PTR)client.get(), &p_over->over);
				}
			}
			g_clients_mutex.unlock();
		}
	}
	catch (Exception& e)
	{
		cout << "Exception! " << e.what() << endl;
		if (!g_is_shutting_down) {
			g_is_shutting_down = true; // 플래그 설정 후 정리
		}

	}
	CloseServer();
	return 0;
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

			XMFLOAT3 p_pos;
			p_pos.x = 8000.f + static_cast<float>(rand() % 150);
			p_pos.z = 8000.f + static_cast<float>(rand() % 150);
			p_pos.y = Terrain::terrain->GetHeight(p_pos.x, p_pos.z);
			remoteClient->respawn_postion = p_pos;
			remoteClient->SetPosition(p_pos);

			LOGIN_PACKET s_packet;
			s_packet.size = sizeof(LOGIN_PACKET);
			s_packet.type = static_cast<char>(E_PACKET::E_P_LOGIN);
			s_packet.uid = remoteClient->m_id;
			// 내 정보 보내기
			remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_packet));

			POSITION_PACKET s_pospacket;
			s_pospacket.size = sizeof(POSITION_PACKET);
			s_pospacket.type = static_cast<char>(E_PACKET::E_P_POSITION);
			s_pospacket.uid = remoteClient->m_id;
			auto& pos = remoteClient->GetPosition();
			s_pospacket.position.x = pos.x;
			s_pospacket.position.y = pos.y;
			s_pospacket.position.z = pos.z;

			remoteClient->tcpConnection.SendOverlapped(reinterpret_cast<char*>(&s_pospacket));

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

			// 현재 서버 시간 동기화
			remoteClient->SendTimePacket(time_accumulator);

		}

		// 인게임 객체 다 보내기 (나중에 옥트리 이동 시 뷰리스트 적용)
		{
			std::vector<tree_obj*> results; // 시야 범위 내 객체 찾기
			tree_obj p_obj{ -1,remoteClient->GetPosition() };
			Octree::GameObjectOctree.query(p_obj, oct_distance, results);
			std::unordered_set<int> new_vl;
			for (auto& obj : results) new_vl.insert(obj->u_id);
			for (auto& obj : results) {
				if (GameObject::gameObjects[obj->u_id]->is_alive == false) continue;
				if (GameObject::gameObjects[obj->u_id]->Gethp() <= 0) continue;
				remoteClient->SendAddPacket(GameObject::gameObjects[obj->u_id]);
			}	
			for (auto& obj : GameObject::ConstructObjects) {
				remoteClient->SendStructPacket(obj);
			}
			remoteClient->vl_mu.lock();
			remoteClient->viewlist = new_vl;
			remoteClient->vl_mu.unlock();
		}
		auto p_obj = std::make_unique<tree_obj>(remoteClient->m_id, remoteClient->GetPosition());
		Octree::PlayerOctree.insert(std::move(p_obj));
		{
			std::lock_guard<std::mutex> lock(remoteClient->c_mu);
			remoteClient->state = PC_INGAME;
		}

		{
			EVENT ev(EVENT_TYPE::E_P_REGENERATE_HP, remoteClient->m_id, -1);
			EVENT::add_timer(ev, 1000); // 1초 후 hp 회복
			ev.e_type = EVENT_TYPE::E_P_REGENERATE_STAMINA;
			EVENT::add_timer(ev, 500); // 0.5초 후 stamina 회복
			ev.e_type = EVENT_TYPE::E_P_CONSUME_HUNGER;
			EVENT::add_timer(ev, 5000); // 5초마다 허기 소모
			ev.e_type = EVENT_TYPE::E_P_CONSUME_THIRST;
			EVENT::add_timer(ev, 4000); // 4초마다 갈증 소모
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
	float spawnmin = 500, spawnmax = 10000;
	float objectMinSize = 15, objectMaxSize = 20;

	int obj_id = 0;
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetPosition(8000.f, Terrain::terrain->GetHeight(8000.f, 8450.f), 8450.f);
		obj->SetScale(10.f, 10.f, 10.f);
		obj->SetID(obj_id++);
		obj->SetType(OBJECT_TYPE::ST_HELIPAD);
		obj->SetAnimationType(ANIMATION_TYPE::UNKNOWN);
		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetPosition(8000.f, Terrain::terrain->GetHeight(8000.f, 8500.f), 8500.f);
		obj->SetScale(10.f, 10.f, 10.f);
		obj->SetID(obj_id++);
		obj->SetType(OBJECT_TYPE::ST_ANTHENA);
		obj->SetAnimationType(ANIMATION_TYPE::UNKNOWN);
		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int TreeCount = 700;
	for (int i = 0; i < TreeCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT || InHeilpad(randompos.first, randompos.second)) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
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
	int RockCount = 700;
	for (int i = 0; i < RockCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT || InHeilpad(randompos.first, randompos.second)) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
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

	int CowCount = 150;
	for (int i = 0; i < CowCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(12.f, 12.f, 12.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_COW);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int PigCount = 150;
	for (int i = 0; i < PigCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(10.f, 10.f, 10.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_PIG);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int SpiderCount = 140;
	for (int i = 0; i < SpiderCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(8.f, 8.f, 8.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_SPIDER);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int WolfCount = 140;
	for (int i = 0; i < WolfCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(10.f, 10.f, 10.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_WOLF);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int ToadCount = 100;
	for (int i = 0; i < ToadCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(8.f, 8.f, 8.f);
		obj->SetID(obj_id++);

		obj->SetType(OBJECT_TYPE::OB_TOAD);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}
	int BatCount = 40;
	for (int i = 0; i < BatCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->fly_height = 13.f;
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second) + obj->fly_height, randompos.second);
		obj->SetScale(9.f, 9.f, 9.f);
		obj->SetID(obj_id++);
		obj->SetType(OBJECT_TYPE::OB_BAT);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	int RaptorCount = 40;
	for (int i = 0; i < RaptorCount; ++i) {
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		}
		obj->SetPosition(randompos.first, Terrain::terrain->GetHeight(randompos.first, randompos.second), randompos.second);
		obj->SetScale(9.f, 9.f, 9.f);
		obj->SetID(obj_id++);
		obj->_hp = 40;
		obj->_atk = 7;
		obj->SetType(OBJECT_TYPE::OB_RAPTOR);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));
	}

	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		//std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		//while (Terrain::terrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
		//	randompos = genRandom::generateRandomXZ(gen, spawnmin, spawnmax, spawnmin, spawnmax);
		//}
		obj->SetPosition(9000, Terrain::terrain->GetHeight(9000, 6000), 6000);
		obj->SetScale(40.f, 40.f, 40.f);
		obj->SetID(obj_id++);
		obj->_fMaxHp = obj->_hp = 100;
		obj->_atk = 50;
		obj->SetType(OBJECT_TYPE::OB_GOLEM);
		obj->SetAnimationType(ANIMATION_TYPE::IDLE);

		obj->InitFSM();
		obj->FSM_manager->SetCurrentState(std::make_shared<BossStandingState>());
		obj->FSM_manager->SetGlobalState(std::make_shared<BossGlobalState>());

		GameObject::gameObjects.push_back(obj);

		auto t_obj = std::make_unique<tree_obj>(obj->GetID(), obj->GetPosition());
		Octree::GameObjectOctree.insert(std::move(t_obj));

	}

	for (auto& obj : GameObject::gameObjects) {
		auto it = OBB_Manager::obb_list.find(obj->GetType());
		if (it != OBB_Manager::obb_list.end()) {
			obj->local_obb = *it->second;
			obj->UpdateTransform();
		}
	}
}
void ReleaseObject()
{
	GameObject::gameObjects.clear();
	GameObject::ConstructObjects.clear();
	Octree::GameObjectOctree.clear();
	//Octree::PlayerOctree.clear();
}


void CloseServer()
{
	cout << "서버의 모든 리소스를 정리하고 종료합니다." << endl;

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
	PlayerClient::PlayerClients.clear(); // 클라이언트 목록 비우기

	cout << "서버가 성공적으로 종료되었습니다." << endl;
}