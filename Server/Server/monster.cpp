#include "global.h"
#include "monster.h"

void Monster::do_recv()
{
}

void Monster::do_send(void* packet)
{
	OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
}

void Monster::send_move_packet(int c_id, SESSION* clients, unsigned move_time)
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

void Monster::send_login_info_packet()
{
}

void Monster::add_session_packet(int c_id, SESSION* clients)
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

void Monster::send_remove_session_packet(int c_id)
{
//	_vl.lock();
	if (_view_list.count(c_id))
		_view_list.erase(c_id);
	else {
	//	_vl.unlock();

		return;
	}
//	_vl.unlock();
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	do_send(&p);
}

//int API_SendMessage(lua_State* L)
//{
//	int my_id = (int)lua_tointeger(L, -3);
//	int user_id = (int)lua_tointeger(L, -2);
//	char* mess = (char*)lua_tostring(L, -1);
//
//	lua_pop(L, 4);
//
//	
//	return 0;
//}



