#include "global.h"
#include "monster.h"

void Monster::do_recv()
{
}

void Monster::do_send(void* packet)
{
}

void Monster::send_move_packet(int c_id, SESSION* clients, unsigned move_time)
{
}

void Monster::send_login_info_packet()
{
}

void Monster::add_session_packet(int c_id, SESSION* clients)
{
}

void Monster::send_remove_session_packet(int c_id)
{
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



