#pragma once
#include "Session.h"
#include "Obstacle.h"


enum MONSTER_TYPE{TY_PEACE, TY_WANDER};
enum CURRENT_STATE { CST_PEACE, CST_CHASE };

class Monster : public SESSION
{
	bool up_chance = false;
	bool down_chance= false;
public:
	atomic_bool	_is_active;		// 주위에 플레이어가 있는가?
	atomic_bool _is_die;
	MONSTER_TYPE _type;
	CURRENT_STATE _current_state;


	int scape_x;
	int scape_y;

public:
	virtual void do_recv() override;
	virtual void do_send(void* packet) override;
	virtual void send_move_packet(int c_id, SESSION* clients, unsigned move_time)override;
	virtual void send_login_info_packet() override;
	virtual void add_session_packet(int c_id, SESSION* clients) override;
	virtual void send_remove_session_packet(int c_id) override;
	//void find_root(SESSION* client, const array<Obstacle, MAX_OBSTACLE>* obstacles);
	void chase_move(SESSION* client, const array<Obstacle, MAX_OBSTACLE>* obstacles);

};


