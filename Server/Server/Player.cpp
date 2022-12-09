#include"global.h"
#include "Player.h"
#include "Over_exp.h"



void Player::do_recv()
{
	DWORD recv_flag = 0;
	memset(&_recv_over._over, 0, sizeof(_recv_over._over));
	_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;//_prev_remain�̰� 0�� �ƴϸ� �������� �̾ �޾ƾ���
	//��� size��ŭ ���ֱ� 
	_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
	//op�ٲ���ߵǳ� 

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
	cout << "dddfs" << endl;
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
