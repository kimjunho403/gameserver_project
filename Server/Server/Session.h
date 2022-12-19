#pragma once
#include "Over_exp.h"
enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };

class SESSION abstract
{
protected:
	OVER_EXP _recv_over;
public:

public:
	
	int _id;
	mutex _s_lock;
	S_STATE _state;
	SOCKET _socket;
	short _x, _y;
	char _name[NAME_SIZE];
	int _prev_remain; //��Ŷ �������ϱ� ���� ���� ���� ��Ŷ
	unsigned last_movetime;
	mutex _vl;
	unordered_set <int> _view_list;
	lua_State* _L;
	mutex	_ll;
public:
	int _hp, _max_hp;
	int _exp, _max_exp;
	short _level;
	short _dir;
	int _power;

public:
	SESSION();
	virtual ~SESSION();
	virtual  void do_recv() = 0;
	virtual void do_send(void* packet) = 0;
	virtual void send_move_packet(int c_id,SESSION* clients) = 0;
	virtual void send_login_info_packet() = 0;
	virtual void add_session_packet(int c_id, SESSION* clients) = 0;
	virtual void send_remove_session_packet(int c_id) = 0;
	
};