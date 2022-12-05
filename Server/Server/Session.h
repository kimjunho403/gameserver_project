#pragma once
#include "Over_exp.h"
enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };

class SESSION abstract
{
protected:
	OVER_EXP _recv_over;

public:
	int _id;
	mutex _s_lock;
	S_STATE _state;
	SOCKET _socket;
	short _x, _y;
	char _name[NAME_SIZE];
	int _prev_remain; //패킷 재조립하기 위한 변수 이전 패킷

public:
	SESSION();
	virtual ~SESSION();
	virtual  void do_recv() = 0;
	virtual void do_send(void* packet) = 0;
	virtual void send_move_packet(int c_id,SESSION* clients) = 0;
	virtual void send_login_info_packet() = 0;
};