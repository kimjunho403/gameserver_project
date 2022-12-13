#include"global.h"
#include "Player.h"
#include "Over_exp.h"



void Player::do_recv()
{
	DWORD recv_flag = 0;
	memset(&_recv_over._over, 0, sizeof(_recv_over._over));
	_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;//_prev_remain이게 0이 아니면 다음부터 이어서 받아야함
	//찌꺼기 size만큼 빼주기 
	_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
	//op바꿔줘야되나 

	WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
}

void Player::do_send(void* packet)
{
	OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
}

void Player::send_move_packet(int c_id, SESSION* clients, unsigned move_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = clients->_x;
	p.y = clients->_y;
	p.dir = clients->_dir;
	p.move_time = move_time;
	do_send(&p);
}

void Player::send_login_info_packet()
{
	SC_LOGIN_INFO_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
	p.x = _x;
	p.y = _y;
	p.hp = _hp;
	p.level = _level;
	p.exp = _exp;

	strcpy_s(p.name,_name);
	do_send(&p);
}

void Player::add_session_packet(int c_id, SESSION* clients)
{
	SC_ADD_OBJECT_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients->_name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_OBJECT;
	add_packet.x = clients->_x;
	add_packet.y = clients->_y;
	add_packet.hp = clients->_hp;
	add_packet.level = clients->_level;
	_vl.lock();
	_view_list.insert(c_id);
	_vl.unlock();

	do_send(&add_packet);
}

void Player::send_remove_session_packet(int c_id)
{
	_vl.lock();
	if (_view_list.count(c_id))
		_view_list.erase(c_id);
	else {
		_vl.unlock();
		
		return;
	}
	_vl.unlock();
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	do_send(&p);
}

void Player::send_obstacle_pos_packet(Obstacle* _obstacle)
{
	SC_OBSTACLE_PACKET p;
	p.id = _obstacle->_id;
	p.size = sizeof(SC_OBSTACLE_PACKET);
	p.type = SC_OBSTACLE;
	p.x = _obstacle->_x;
	p.y = _obstacle->_y;
	do_send(&p);
}

void Player::send_player_info_packet(int c_id)
{
	SC_PLAYERINFO_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_PLAYERINFO_PACKET);
	p.type = SC_PLAYERINFO;
	p.hp = _hp;
	p.max_hp = _hp;
	p.exp = _exp;
	p.level = _level;

	do_send(&p);
}

void Player::send_player_attack_packet(int c_id)
{
	SC_PLAYERATTACK_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_PLAYERATTACK_PACKET);
	p.type = SC_PLAYERATTACK;

	do_send(&p);
}

void Player::send_monster_hp_packet(int c_id, int hp)
{
	SC_MONSTER_HP_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MONSTER_HP_PACKET);
	p.type = SC_MONSTERHP;
	p.hp = hp;
	do_send(&p);
}


