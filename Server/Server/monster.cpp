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

void Monster::send_move_packet(int c_id, SESSION* clients)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = clients->_x;
	p.y = clients->_y;
	p.dir = clients->_dir;
	p.move_time = last_movetime;
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

//void Monster::find_root(SESSION* client, const array<Obstacle, MAX_OBSTACLE>* obstacles)
//{
//	int x = _x;
//	int y = _y;
//	cout << "find" << endl;
//	while (!q_root.empty())   q_root.pop();//큐 초기화
//
//	while (client->_x != x && client->_y != y) {
//		if (x < client->_x) {
//			x++;
//			for (auto& ob : *obstacles) {
//				if (x == ob._x && y == ob._y) {
//					x--;
//					y++;
//					q_root.push(DOWN);
//					break;
//				}
//			}
//
//			q_root.push(RIGHT);
//		}
//		else if (x > client->_x) {
//			x--;
//			for (auto& ob : *obstacles) {
//				if (x == ob._x && y == ob._y) {
//					x++;
//					y--;
//					q_root.push(UP);
//					break;
//				}
//			}
//			q_root.push(LEFT);
//		}
//		else if (y < client->_y) {
//			y++;
//			for (auto& ob : *obstacles) {
//				if (x == ob._x && y == ob._y) {
//					y--;
//					x++;
//					q_root.push(RIGHT);
//					break;
//				}
//			}
//			q_root.push(DOWN);
//		}
//		else if (y > client->_y) {
//			y--;
//			for (auto& ob : *obstacles) {
//				if (x == ob._x && y == ob._y) {
//					y++;
//					x--;
//					q_root.push(LEFT);
//					break;
//				}
//			}
//			q_root.push(UP);
//		}
//	}
//	cout << "end" << endl;
//}

void Monster::chase_move(SESSION* client,const array<Obstacle,MAX_OBSTACLE>* obstacles)
{
	//switch (q_root.front())
	//{
	//case LEFT:
	//	--_x;
	//	break;
	//case RIGHT:
	//	--_x;
	//	break;
	//case UP:
	//	--_y;
	//	break;
	//case DOWN:
	//	++_y;
	//	break;
	//} 
	//q_root.pop();
	if (up_chance == true) {
		_y--;
		up_chance = false;
		return;
	}
	if (down_chance == true) {
		_y++;
		down_chance = false;
		return;
	}
	if (_x < client->_x) {
		_x++;
		for (auto& ob : *obstacles) {
			if (_x == ob._x && _y == ob._y) {
				--_x;
				++_y;
			}

		}
	}
	else if(_x > client->_x) {
		_x--;
		for (auto& ob : *obstacles) {
			if (_x == ob._x && _y == ob._y) {
				++_x;
				--_y;
			}
		}
	}
	else if (_y < client->_y) {
		_y++;
		for (auto& ob : *obstacles) {
			if (_x == ob._x && _y == ob._y) {
				--_y;
				--_x;
				down_chance = true;
			}
		}
	}
	else if (_y > client->_y ) {
		_y--;
		for (auto& ob : *obstacles) {
			if (_x == ob._x && _y == ob._y) {
				++_y;
				++_x;
				up_chance = true;
			}
		}
	}
}







