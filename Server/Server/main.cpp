#include "global.h"
#include "Over_exp.h"
#include "Session.h"
#include "Player.h"
OVER_EXP g_a_over;//accept�� �������� ����ü ��� data rece�� �ƴ� 
//���ÿ� 2���� �������尡 �Ϸ�ɸ��� ���� 
SOCKET g_c_SOCKET;
SOCKET g_s_SOCKET;

array < SESSION*, MAX_USER + MAX_NPC > clients; //������Ʈ Ǯ
unordered_set<int> login_index;
int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {

		/*lock_guard<mutex>ll( clients[i]->_s_lock );*/
		if (clients[i]== nullptr)
		{
			clients[i] =new Player;
			return i;
		}
	}
	return -1;
}

void disconnect(int c_id)
{
	for (auto& index : login_index) {
		//{
		//	lock_guard<mutex> ll(pl->_s_lock);
			if (ST_INGAME != clients[index]->_state) continue;//�� Ŭ�� ������̸� �˷����ʿ䰡����
	//	}
		if (clients[index]->_id == c_id) continue;
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		clients[index]->do_send(&p);
	}
	closesocket(clients[c_id]->_socket);

	//lock_guard<mutex> ll(clients[c_id]->_s_lock);
	clients[c_id]->_state = ST_FREE;
	login_index.erase(c_id);
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id/*�갡 �α����Ѱ� �˷���*/]->_name, p->name);
		clients[c_id]->send_login_info_packet();
		{
			lock_guard<mutex> ll(clients[c_id]->_s_lock);
			clients[c_id]->_state = ST_INGAME;

			login_index.insert(c_id);
		}
		for (auto& index : login_index) {
			//{
			//	lock_guard<mutex> ll(pl->_s_lock);
			//	if (ST_INGAME != pl->_state) continue;//�� Ŭ�� ������̸� �˷����ʿ䰡����
			//}
		//	if (pl == nullptr) continue;
			if (clients[index]->_id == c_id) continue;//�� �ڽſ��� �����ʿ�� ���� 
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = c_id;
			strcpy_s(add_packet.name, clients[index]->_name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.x = clients[c_id]->_x;
			add_packet.y = clients[c_id]->_y;
			clients[index]->do_send(&add_packet);
		}
		for (auto& index : login_index) {
			//{
			//	lock_guard<mutex> ll(pl->_s_lock);
			//	if (ST_INGAME != pl->_state) continue;//�� Ŭ�� ������̸� �˷����ʿ䰡����
			//}
			//if (pl == nullptr) continue;
			if (clients[index]->_id == c_id) continue;
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = clients[index]->_id;
			strcpy_s(add_packet.name, clients[index]->_name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.x = clients[index]->_x;
			add_packet.y = clients[index]->_y;
			clients[c_id]->do_send(&add_packet);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = clients[c_id]->_x;
		short y = clients[c_id]->_y;
		switch (p->direction) {
		case 0: if (y > 0) y--;     break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		clients[c_id]->_x = x;
		clients[c_id]->_y = y;



		for (auto& index : login_index) {
			/*{
					lock_guard<mutex> ll (clients[index]->_s_lock);
				if (ST_INGAME != clients[index]->_state) continue;
			}*/
			clients[index]->send_move_packet(c_id,clients[c_id]);
		}


		break;
	}
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
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);//�Ϸ�Ȱ� ������ ó�� 
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));//disconnet ���̻� ��Ŷ�� ������ �����ϱ� 
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();//���ο� Ŭ�� ���̵� ���� 
			if (client_id != -1) {//-1�̸� ���̻� ������ ���� 
				/*{
					lock_guard<mutex> ll(clients[client_id]->_s_lock);
					clients[client_id]->_state = ST_ALLOC;
				}*/
				clients[client_id]->_x = rand() % 400;
				clients[client_id]->_y = rand() % 400;
				clients[client_id]->_id = client_id;
				clients[client_id]->_name[0] = 0;
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
			int remain_data = num_bytes + clients[key]->_prev_remain;//�������� ó���������� ��Ŷ�� 
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);//��Ŷ ������
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]->_prev_remain = remain_data;//��� ��Ŷ �Ǿ����� �̵�
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key]->do_recv();

			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		}

	}
}

int main()
{
	HANDLE h_iocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_SOCKET = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); //���� ���� ���� 
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_SOCKET, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_SOCKET, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);


	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);//iocp ��� iocp�ڵ鸸��� 
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_SOCKET), h_iocp, 9999/*������ ������ ���⶧���� ����ó��*/, 0);//
	g_c_SOCKET = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);//Ŭ�� ���� ����

	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_SOCKET, g_c_SOCKET, g_a_over._send_buf/*char ����*/, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	vector<thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i) {
		worker_threads.emplace_back(worker_thread, h_iocp);
	}
	for (auto& th : worker_threads) {
		th.join();
	}

	closesocket(g_s_SOCKET);
	WSACleanup();
}
