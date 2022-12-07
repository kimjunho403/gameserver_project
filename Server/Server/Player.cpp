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
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
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
