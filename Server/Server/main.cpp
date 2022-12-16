#include "global.h"
#include "Over_exp.h"
#include "Session.h"
#include "Player.h"
#include "monster.h"
#include "Obstacle.h"
#include "DB.h"
#include <random>

enum EVENT_TYPE { EV_RANDOM_MOVE, EV_RESPAWN, EV_HPUP, EV_CHASE };

struct TIMER_EVENT {
	int obj_id;
	chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	int target_id;
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};

concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

OVER_EXP g_a_over;//accept용 오버랩드 구조체 얘는 data rece가 아님 
//동시에 2개의 오버랩드가 완료될리가 없음 
SOCKET g_c_SOCKET;
SOCKET g_s_SOCKET;


HANDLE h_iocp;
array < SESSION*, MAX_USER + MAX_NPC > clients; //오브젝트 풀
array < Obstacle, MAX_OBSTACLE > obsatcles;
DB User_DB;



void init_obstacle() {
	cout << "obstacle init" << endl;
	int i = 0;
	for (int x = 500; x < 1500; x++) {
		for (int y = 500; y < 1500; y++) {
			if (rand() % 50 == 1 && i <= MAX_OBSTACLE) {
				obsatcles[i]._id = i;
				obsatcles[i]._x = x;
				obsatcles[i]._y = y;
				i++;
			}
		}
	}
	cout << "obstacle init end" << endl;
	//for (auto& _ob : obsatcles) {
	//	_ob._id = i;
	//	_ob._x = rand() % 2000;
	//	_ob._y = rand() % 2000;
	//	i++;

	//}
}
bool is_collision(SESSION* player, short dir) {
	switch (dir)
	{
	case LEFT:
		for (auto& _ob : obsatcles) {
			if (_ob._x == player->_x - 1 && _ob._y == player->_y) {
				return true;
			}
		}
		break;
	case RIGHT:
		for (auto& _ob : obsatcles) {
			if (_ob._x == player->_x + 1 && _ob._y == player->_y) {
				return true;
			}
		}
		break;
	case UP:
		for (auto& _ob : obsatcles) {
			if (_ob._x == player->_x && _ob._y == player->_y + 1) {
				return true;
			}
		}
		break;
	case DOWN:
		for (auto& _ob : obsatcles) {
			if (_ob._x == player->_x && _ob._y == player->_y - 1) {
				return true;
			}
		}
		break;
	}
	return false;
}
bool can_see(int c1, int c2)
{
	if (abs(clients[c1]->_x - clients[c2]->_x) > VIEW_RANGE) return false;
	return abs(clients[c1]->_y - clients[c2]->_y) <= VIEW_RANGE;

}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]->_s_lock };
		if (clients[i]->_state == ST_FREE)
			return i;
	}
	return -1;
}

void disconnect(int c_id)
{
	clients[c_id]->_vl.lock();
	unordered_set <int> vl = clients[c_id]->_view_list;
	clients[c_id]->_vl.unlock();
	for (auto& p_id : vl) {
		if (p_id > MAX_USER) continue;
		auto& pl = clients[p_id];
		{
			lock_guard<mutex> ll((pl->_s_lock));
			if (ST_INGAME != pl->_state) continue;//이 클라가 사용중이면 알려줄필요가없음
		}
		if (pl->_id == c_id) continue;
		pl->send_remove_session_packet(c_id);
	}
	closesocket(clients[c_id]->_socket);

	lock_guard<mutex> ll(clients[c_id]->_s_lock);
	clients[c_id]->_state = ST_FREE;
#ifdef STRESS_TEST

#else
	User_DB.update_db(clients[c_id]->_name, clients[c_id]->_x, clients[c_id]->_y, clients[c_id]->_level, clients[c_id]->_hp, clients[c_id]->_exp);
#endif
}

void do_npc_random_move(int npc_id)
{
	SESSION& npc = *clients[npc_id];
	unordered_set<int> old_vl;
	for (auto& obj : clients) {//npc기준 시야에 닿는 유저 뷰리스트에 유저 추가
		if (ST_INGAME != obj->_state) continue;
		if (true == obj->_id > MAX_USER) continue;
		if (true == can_see(npc._id, obj->_id))
			old_vl.insert(obj->_id);
	}

	//int x = npc._x;
	//int y = npc._y;
	//switch (rand() % 4) {
	//case 0: if (x < (W_WIDTH - 1)) x++; break;
	//case 1: if (x > 0) x--; break;
	//case 2: if (y < (W_HEIGHT - 1)) y++; break;
	//case 3:if (y > 0) y--; break;
	//}
	//npc._x = x;
	//npc._y = y;

	unordered_set<int> new_vl;
	for (auto& obj : clients) {//좌표 변환후 뷰 리스트에 유저 추가
		if (ST_INGAME != obj->_state) continue;
		if (true == obj->_id > MAX_USER) continue;
		if (true == can_see(npc._id, obj->_id))
			new_vl.insert(obj->_id);
	}

	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			// 플레이어의 시야에 등장
			clients[pl]->add_session_packet(npc._id, clients[npc._id]);
		}
		else {
			// 플레이어가 계속 보고 있음.
			clients[pl]->send_move_packet(npc._id, clients[npc._id], clients[npc._id]->last_movetime);
		}
	}
	///시야에 있었다가 사라짐
	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			clients[pl]->_vl.lock();
			if (0 != clients[pl]->_view_list.count(npc._id)) {
				clients[pl]->_vl.unlock();
				clients[pl]->send_remove_session_packet(npc._id);
			}
			else {
				clients[pl]->_vl.unlock();
			}
		}
	}
}

void do_ai_chase_move(int ai_id,int target_id)
{
	SESSION& npc = *clients[ai_id];
	unordered_set<int> old_vl;
	for (auto& obj : clients) {//npc기준 시야에 닿는 유저 뷰리스트에 유저 추가
		if (ST_INGAME != obj->_state) continue;
		if (true == obj->_id > MAX_USER) continue;
		if (true == can_see(npc._id, obj->_id))
			old_vl.insert(obj->_id);
	}
	reinterpret_cast<Monster*>(&npc)->chase_move(clients[target_id]);

	//int x = npc._x;
	//int y = npc._y;
	//switch (rand() % 4) {
	//case 0: if (x < (W_WIDTH - 1)) x++; break;
	//case 1: if (x > 0) x--; break;
	//case 2: if (y < (W_HEIGHT - 1)) y++; break;
	//case 3:if (y > 0) y--; break;
	//}
	//npc._x = x;
	//npc._y = y;

	unordered_set<int> new_vl;
	for (auto& obj : clients) {//좌표 변환후 뷰 리스트에 유저 추가
		if (ST_INGAME != obj->_state) continue;
		if (true == obj->_id > MAX_USER) continue;
		if (true == can_see(npc._id, obj->_id))
			new_vl.insert(obj->_id);
	}

	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			// 플레이어의 시야에 등장
			clients[pl]->add_session_packet(npc._id, clients[npc._id]);
		}
		else {
			// 플레이어가 계속 보고 있음.
			clients[pl]->send_move_packet(npc._id, clients[npc._id], clients[npc._id]->last_movetime);
		}
	}
	///시야에 있었다가 사라짐
	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			clients[pl]->_vl.lock();
			if (0 != clients[pl]->_view_list.count(npc._id)) {
				clients[pl]->_vl.unlock();
				clients[pl]->send_remove_session_packet(npc._id);
			}
			else {
				clients[pl]->_vl.unlock();
			}
		}
	}
}

void WakeUpNPC(int npc_id, int waker)
{
	OVER_EXP* exover = new OVER_EXP;
	exover->_comp_type = OP_AI_HELLO;
	exover->_ai_target_obj = waker;
	PostQueuedCompletionStatus(h_iocp, 1, npc_id, &exover->_over);

	if (reinterpret_cast<Monster*>(clients[npc_id])->_is_active) return;
	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&reinterpret_cast<Monster*>(clients[npc_id])->_is_active, &old_state, true))
		return;
	TIMER_EVENT ev{ npc_id, chrono::system_clock::now(), EV_RANDOM_MOVE, 0 };
	timer_queue.push(ev);
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		//이미 로그인 한 플레이어인지 체크 
		for (auto& cl : clients) {
			if (0 == strcmp(cl->_name, p->name) && cl->_state == ST_INGAME) {
				reinterpret_cast<Player*>(clients[c_id])->send_login_fail_packet();
				disconnect(c_id);
				return;
		}
			if (cl->_id > MAX_USER) break;
			
			
		}

		strcpy_s(clients[c_id/*얘가 로그인한걸 알려줌*/]->_name, p->name);

#ifdef STRESS_TEST
		{
			lock_guard<mutex> ll{ clients[c_id]->_s_lock };
			clients[c_id]->_x = rand() % W_WIDTH;
			clients[c_id]->_y = rand() % W_HEIGHT;
			cout << clients[c_id]->_x << "        " << clients[c_id]->_y << endl;
		}
#else
	

		if (false == User_DB.check_id(clients[c_id]->_name, clients[c_id]->_x, clients[c_id]->_y, clients[c_id]->_level, clients[c_id]->_hp, clients[c_id]->_exp))//없는 아이디 
		{
			User_DB.add_user(clients[c_id]->_name, clients[c_id]->_x, clients[c_id]->_y, clients[c_id]->_level, clients[c_id]->_hp, clients[c_id]->_exp);
			User_DB.check_id(clients[c_id]->_name, clients[c_id]->_x, clients[c_id]->_y, clients[c_id]->_level, clients[c_id]->_hp, clients[c_id]->_exp);
			clients[c_id]->_power = clients[c_id]->_level * 5;
			clients[c_id]->_max_exp = clients[c_id]->_level * 100; ;
		}
		else {
			clients[c_id]->_power = clients[c_id]->_level * 5;
			clients[c_id]->_max_exp = clients[c_id]->_level * 100; ;

		}

		cout << clients[c_id]->_x << "   " << clients[c_id]->_y << endl;
#endif
		{
			lock_guard<mutex> ll(clients[c_id]->_s_lock);
			clients[c_id]->_state = ST_INGAME;
		}
		clients[c_id]->send_login_info_packet(); //나한테 

		for (auto& cl : clients) {//내 정보를 다른 클라한테 정보 넘기자
			{
				lock_guard<mutex> ll(cl->_s_lock);
				if (ST_INGAME != cl->_state) continue;//이 클라가 사용중이면 알려줄필요가없음
			}
			if (cl->_id == c_id) continue;//내 자신에게 보낼필요는 없음 
			if (false == can_see(c_id, cl->_id)) continue;
			if (cl->_id < MAX_USER)cl->add_session_packet(c_id, clients[c_id]);
			else WakeUpNPC(cl->_id, c_id);
			clients[c_id]->add_session_packet(cl->_id, cl);
		}
		//장애물 위치 
		for (auto& ob : obsatcles) {
			static_cast<Player*>(clients[c_id])->send_obstacle_pos_packet(&ob);
		}

		OVER_EXP* exover = new OVER_EXP;
		exover->_comp_type = OP_PLAYER_HPUP;
		PostQueuedCompletionStatus(h_iocp, 1, c_id, &exover->_over);


		TIMER_EVENT ev{ c_id, chrono::system_clock::now() + 5s, EV_RANDOM_MOVE, 0 };
		timer_queue.push(ev);

		break;
	}
	case CS_LOGOUT:
	{
		disconnect(c_id);

		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id]->last_movetime = p->move_time;
		short x = clients[c_id]->_x;
		short y = clients[c_id]->_y;
		if (!is_collision(clients[c_id], p->direction)) {

			switch (p->direction) {
			case DOWN: if (y > 0) y--; clients[c_id]->_dir = DOWN;  break;
			case UP: if (y < W_HEIGHT - 1) y++; clients[c_id]->_dir = UP; break;
			case LEFT: if (x > 0) x--; clients[c_id]->_dir = LEFT; break;
			case RIGHT: if (x < W_WIDTH - 1) x++; clients[c_id]->_dir = RIGHT; break;
			}

		}

		clients[c_id]->_x = x;
		clients[c_id]->_y = y;

		unordered_set<int> near_list;
		clients[c_id]->_vl.lock();
		unordered_set<int> old_vlist = clients[c_id]->_view_list;
		clients[c_id]->_vl.unlock();
		for (auto& cl : clients) {
			if (ST_INGAME != cl->_state) continue;
			if (cl->_id == c_id) continue;
			if (can_see(c_id, cl->_id)) near_list.insert(cl->_id);

		}
		clients[c_id]->send_move_packet(c_id, clients[c_id], clients[c_id]->last_movetime);

		//다른애들한테 내 정보 전달 
		for (auto& near_pl : near_list) {
			auto& cpl = clients[near_pl];
			if (clients[near_pl]->_id < MAX_USER) {
				cpl->_vl.lock();
				if (clients[near_pl]->_view_list.count(c_id)) {
					cpl->_vl.unlock();
					clients[near_pl]->send_move_packet(c_id, clients[c_id], clients[c_id]->last_movetime);
				}
				else
				{
					cpl->_vl.unlock();
					clients[near_pl]->add_session_packet(c_id, clients[c_id]);
				}
			}
			else WakeUpNPC(near_pl, c_id);

			if (old_vlist.count(near_pl) == 0)
				clients[c_id]->add_session_packet(near_pl, clients[near_pl]);

		}
		//viewlist에서 사라졌다면 remove 패킷 전달 
		for (auto& index : old_vlist) {
			if (0 == near_list.count(index)) {
				clients[c_id]->send_remove_session_packet(index);
				clients[index]->send_remove_session_packet(c_id);
			}

		}
		break;
	}
	case CS_ATTACK: {
		//CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);

		clients[c_id]->_vl.lock();
		unordered_set<int> old_vlist = clients[c_id]->_view_list;
		clients[c_id]->_vl.unlock();

		//npc 뷰리스트에서 공격체크 
		for (auto& near_pl : old_vlist) {
			auto& cpl = clients[near_pl];
			if (clients[near_pl]->_id > MAX_USER) {
				switch (clients[c_id]->_dir)
				{
				case LEFT:
					if (clients[near_pl]->_x + 1 == clients[c_id]->_x && clients[near_pl]->_y == clients[c_id]->_y) {
						//clients[near_pl]->_hp -= clients[c_id]->_power; // 체력 -
						cpl->_ll.lock();
						lua_getglobal(clients[near_pl]->_L, "set_hp");

						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_power);
						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_id);

						if (lua_pcall(clients[near_pl]->_L, 2, 0, 0))
							printf("Error calling lua function: %s\n", lua_tostring(clients[near_pl]->_L, -1));
						cpl->_ll.unlock();
						// lua_pop(L, 1);// eliminate set_uid from stack after call
						cout << "온스터에게" << clients[c_id]->_power << "피해를 입힘" << endl;
					}
					break;
				case RIGHT:
					if (clients[near_pl]->_x - 1 == clients[c_id]->_x && clients[near_pl]->_y == clients[c_id]->_y) {
						//clients[near_pl]->_hp -= clients[c_id]->_power;
						cpl->_ll.lock();
						lua_getglobal(clients[near_pl]->_L, "set_hp");

						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_power);
						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_id);
						if (lua_pcall(clients[near_pl]->_L, 2, 0, 0))
							printf("Error calling lua function: %s\n", lua_tostring(clients[near_pl]->_L, -1));
						cpl->_ll.unlock();
						cout << "온스터에게" << clients[c_id]->_power << "피해를 입힘" << endl;
					}
					break;
				case UP:
					if (clients[near_pl]->_x == clients[c_id]->_x && clients[near_pl]->_y - 1 == clients[c_id]->_y) {
						//clients[near_pl]->_hp -= clients[c_id]->_power;
						cpl->_ll.lock();
						lua_getglobal(clients[near_pl]->_L, "set_hp");

						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_power);
						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_id);
						if (lua_pcall(clients[near_pl]->_L, 2, 0, 0))
							printf("Error calling lua function: %s\n", lua_tostring(clients[near_pl]->_L, -1));
						cpl->_ll.unlock();
						cout << "온스터에게" << clients[c_id]->_power << "피해를 입힘" << endl;
					}
					break;
				case DOWN:
					if (clients[near_pl]->_x == clients[c_id]->_x && clients[near_pl]->_y + 1 == clients[c_id]->_y) {
						//clients[near_pl]->_hp -= clients[c_id]->_power;
						cpl->_ll.lock();
						lua_getglobal(clients[near_pl]->_L, "set_hp");

						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_power);
						lua_pushnumber(clients[near_pl]->_L, clients[c_id]->_id);
						if (lua_pcall(clients[near_pl]->_L, 2, 0, 0))
							printf("Error calling lua function: %s\n", lua_tostring(clients[near_pl]->_L, -1));
						cpl->_ll.unlock();
						cout << "온스터에게" << clients[c_id]->_power << "피해를 입힘" << endl;
					}
					break;
				}


			}
			else { //player to player
				reinterpret_cast<Player*>(clients[near_pl])->send_player_attack_packet(c_id);
				//reinterpret_cast<Player*>(clients[c_id])->send_player_attack_packet(near_pl);
			}
			//	else WakeUpNPC(near_pl, c_id);

				/*if (old_vlist.count(near_pl) == 0)
					clients[c_id]->add_session_packet(near_pl, clients[near_pl]);*/

		}
	
	}
				  break;
	}
}

void worker_thread(HANDLE h_iocp) {
	//using FpFloatMilliseconds = duration<float, milliseconds::period>;
	//auto prev_Time = chrono::high_resolution_clock::now();
	//float elapsedTime{};
	//float deltaT;
	while (true) {
		/*auto cur_Time = chrono::high_resolution_clock::now();
		elapsedTime += FpFloatMilliseconds(cur_Time - prev_Time).count();
		deltaT = FpFloatMilliseconds(cur_Time - prev_Time).count();
		prev_Time = cur_Time;
		if (elapsedTime > 1000.0f / 60.0f)
		{*/
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);//완료된게 있으면 처리 
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));//disconnet 더이상 패킷이 올일이 없으니까 
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();//새로온 클라 아이디 받음 
			if (client_id != -1) {//-1이면 더이상 받을수 없음 
				{
					lock_guard<mutex> ll(clients[client_id]->_s_lock);
					clients[client_id]->_state = ST_ALLOC;
				}

				clients[client_id]->_id = client_id;
				clients[client_id]->_prev_remain = 0;
				clients[client_id]->_socket = g_c_SOCKET;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_SOCKET), h_iocp, client_id, 0);
				clients[client_id]->do_recv();
				g_c_SOCKET = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_SOCKET, g_c_SOCKET, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]->_prev_remain;//지난번에 처리하지못한 패킷과 
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);//패킷 재조립
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]->_prev_remain = remain_data;//찌꺼기 패킷 맨앞으로 이동
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key]->do_recv();

			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		case OP_NPC_MOVE: {
			bool keep_alive = false;
			for (int j = 0; j < MAX_USER; ++j) {
				if (clients[j]->_state != ST_INGAME) continue;
				if (can_see(static_cast<int>(key), j)) {
					keep_alive = true;
					break;
				}
			}
			if (true == keep_alive) {
				do_npc_random_move(static_cast<int>(key));
				TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
				timer_queue.push(ev);
			}
			else {
				reinterpret_cast<Monster*>(clients[key])->_is_active = false;
			}
			delete ex_over;
		}
						break;

		case OP_NPC_RESPAWN: {
			clients[key]->_ll.lock();
			auto L = clients[key]->_L;
			//몬스터 정보 불어오기
			lua_getglobal(L, "getrespawn");
			lua_pcall(L, 0, 3, 0);
			int hp = (int)lua_tointeger(L, -3);
			int x = (int)lua_tointeger(L, -2);
			int y = (int)lua_tointeger(L, -1);
			lua_pop(L, 3);
			clients[key]->_ll.unlock();
			clients[key]->_x = x;
			clients[key]->_y = y;
			clients[key]->_hp = hp;
			//시야에 들어오는 플레이어에게 보여주기 
			for (int j = 0; j < MAX_USER; ++j) {
				if (clients[j]->_state != ST_INGAME) continue;
				if (can_see(static_cast<int>(key), j)) {
					clients[j]->add_session_packet(static_cast<int>(key), clients[key]);
				}
			}

			delete ex_over;
		}
						   break;

		case OP_PLAYER_HPUP: {
			if (clients[key]->_level * 100 > clients[key]->_hp)
				clients[key]->_hp += clients[key]->_level;
			else
				clients[key]->_hp = clients[key]->_level * 100;

			reinterpret_cast<Player*>(clients[key])->send_stat_changel_packet();
			//시야에 들어오는 플레이어에게 보여주기 
			TIMER_EVENT ev{ key, chrono::system_clock::now() + 5s, EV_HPUP, 0 };
			timer_queue.push(ev);

			delete ex_over;
		}
						   break;
		case OP_CHASE : {
			bool keep_alive = false;
			for (int j = 0; j < MAX_USER; ++j) {//계속 누군가의 시야에 있다면 keep_alive를 true로
				if (clients[j]->_state != ST_INGAME) continue;
				if (can_see(static_cast<int>(key), j)) {
					keep_alive = true;
					break;
				}
			}

			if (true == keep_alive) {
				do_ai_chase_move(key,ex_over->_ai_target_obj);
				
				//do_npc_random_move(static_cast<int>(key));
				TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_CHASE, 0 };
				timer_queue.push(ev);
			}
			else {
				reinterpret_cast<Monster*>(clients[key])->_is_active = false;
			}

			//여기 스크립트 공격 event
			clients[key]->_ll.lock();
			lua_getglobal(clients[key]->_L, "event_attack");
			lua_pushnumber(clients[key]->_L, ex_over->_ai_target_obj);
			if (lua_pcall(clients[key]->_L, 1, 0, 0))
				printf("Error calling lua function: %s\n", lua_tostring(clients[key]->_L, -1));

			clients[key]->_ll.unlock();

			delete ex_over;
		}
						   break;

		}

	}
}

void InitiallizePlayer() {
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i] = new Player;
	}
}
//스크립트 함수
int API_get_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = clients[user_id]->_x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = clients[user_id]->_y;
	lua_pushnumber(L, y);
	return 1;
}

int API_SendMessage(lua_State* L)
{
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 4);

	//clients[user_id]->send_chat_packet(my_id, mess);
	return 0;
}

int API_MonsterDie(lua_State* L)
{
	cout << "몬스터 죽음" << endl;
	int moster_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	int exp = (int)lua_tointeger(L, -1);

	lua_pop(L, 3);

	clients[user_id]->_exp += exp;

	if (clients[user_id]->_exp >= clients[user_id]->_max_exp) { //레벨 업 
		clients[user_id]->_exp = 0;
		
		clients[user_id]->_level++;
		clients[user_id]->_max_exp = clients[user_id]->_level * 100;
		clients[user_id]->_power = clients[user_id]->_level * 5;
	}
	reinterpret_cast<Player*>(clients[user_id])->send_stat_changel_packet();// 경험치 받은 플레이어 정보 전달 
	//다른 유저한테 얘 죽었다고 알려줘야됨
	for (int i = 0; i < MAX_USER; ++i) {
		if (can_see(moster_id, i)) {
			reinterpret_cast<Player*>(clients[i])->send_remove_session_packet(moster_id);
		}
	}
	//죽은 몬스터 묘지로
	clients[moster_id]->_x = -100;
	clients[moster_id]->_y = -100;

	TIMER_EVENT ev{ moster_id, chrono::system_clock::now() + 10s, EV_RESPAWN, 0 };//10초후 부활해라 타이머 PUSH
	timer_queue.push(ev);

	return 0;
}

int API_MonsterHit(lua_State* L)
{
	cout << "몬스터 맞음" << endl;
	int c_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	int hp = (int)lua_tointeger(L, -1);

	lua_pop(L, 3);

	clients[c_id]->_hp = hp;

	for (int i = 0; i < MAX_USER; ++i) {
		if (can_see(c_id, i)) {
			reinterpret_cast<Player*>(clients[i])->send_monster_hp_packet(c_id, hp);
		}
	}

	TIMER_EVENT ev{ c_id, chrono::system_clock::now() + 1s, EV_CHASE, user_id };//플레이어 따라가라
	timer_queue.push(ev);

	return 0;
}

int API_attack(lua_State* L)
{
	cout << "몬스터 공격" << endl;
	int monster_id = (int)lua_tointeger(L, -2);
	int plyaer_id = (int)lua_tointeger(L, -1);

	lua_pop(L, 2);

	clients[plyaer_id]->_hp -= clients[monster_id]->_level*5;
	if (clients[plyaer_id]->_hp <= 0)//죽음
	{
		clients[plyaer_id]->_exp /= 2;
		clients[plyaer_id]->_x = 1000;
		clients[plyaer_id]->_y = 1000;
		clients[plyaer_id]->_hp = clients[plyaer_id]->_level * 100;
	}
	
	reinterpret_cast<Player*>(clients[plyaer_id])->send_stat_changel_packet();
	

	//TIMER_EVENT ev{ c_id, chrono::system_clock::now() + 1s, EV_CHASE, user_id };//플레이어 따라가라
	//timer_queue.push(ev);

	return 0;
}
void InitializeNPC()
{
	cout << "NPC intialize begin.\n";

	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		clients[i] = new Monster;
		clients[i]->_state = ST_INGAME;

		auto L = clients[i]->_L = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadfile(L, "monster.lua");
		lua_pcall(L, 0, 0, 0);

		lua_getglobal(L, "set_uid");
		lua_pushnumber(L, i);
		lua_pcall(L, 1, 0, 0);

		lua_getglobal(L, "get_info");
		lua_pcall(L, 0, 8, 0);
		int myid = (int)lua_tointeger(L, -8);
		char* name = (char*)lua_tostring(L, -7);
		int level = (int)lua_tointeger(L, -6);
		int hp = (int)lua_tointeger(L, -5);
		int power = (int)lua_tointeger(L, -4);
		int exp = (int)lua_tointeger(L, -3);
		int x = (int)lua_tointeger(L, -2);
		int y = (int)lua_tointeger(L, -1);
		lua_pop(L, 8);

		clients[i]->_x = x;
		clients[i]->_y = y;
		clients[i]->_level = level;
		clients[i]->_hp = hp;
		clients[i]->_power = power;

		clients[i]->_id = i;
		strcpy_s(clients[i]->_name, name);

		// lua_pop(L, 1);// eliminate set_uid from stack after call
		lua_register(L, "API_attack", API_attack);
		lua_register(L, "API_MonsterDie", API_MonsterDie);
		lua_register(L, "API_MonsterHit", API_MonsterHit);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);
	}
	cout << "NPC initialize end.\n";
}

void do_timer()
{

	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		
		if (true == timer_queue.try_pop(ev)) {

			if (ev.wakeup_time > current_time) {//아직 실행시간이 안됐다면
				timer_queue.push(ev);		// 최적화 필요
				// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(1ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.event_id) {
			case EV_RANDOM_MOVE:
			{
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_NPC_MOVE;
				PostQueuedCompletionStatus(h_iocp, 1, ev.obj_id, &ov->_over);
			}
			break;
			case EV_RESPAWN:
			{
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_NPC_RESPAWN;
				PostQueuedCompletionStatus(h_iocp, 1, ev.obj_id, &ov->_over);

				break;
			}
			case EV_HPUP:
			{
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_PLAYER_HPUP;
				PostQueuedCompletionStatus(h_iocp, 1, ev.obj_id, &ov->_over);

				break;
			}
			case EV_CHASE:
			{
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_CHASE;
				ov->_ai_target_obj = ev.target_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.obj_id, &ov->_over);

				break;
			}
			}
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}
}

int main()
{
	srand(time(NULL));
	init_obstacle();
	InitiallizePlayer();
	InitializeNPC();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_SOCKET = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); //서버 전용 소켓 
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_SOCKET, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_SOCKET, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);


	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);//iocp 등록 iocp핸들만들기 
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_SOCKET), h_iocp, 9999/*서버는 세션이 없기때문에 예외처리*/, 0);//
	g_c_SOCKET = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);//클라 전용 소켓

	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_SOCKET, g_c_SOCKET, g_a_over._send_buf/*char 버퍼*/, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	vector<thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i) {
		worker_threads.emplace_back(worker_thread, h_iocp);
	}
	thread timer_thread{ do_timer };
	timer_thread.join();
	for (auto& th : worker_threads) {
		th.join();
	}

	closesocket(g_s_SOCKET);
	WSACleanup();
}
