#pragma once
#include "Session.h"
#include "Obstacle.h"
class Player : public SESSION
{
public:
	bool _is_buff;
public:
	virtual void do_recv() override;
	virtual void do_send(void* packet) override;
	virtual void send_move_packet(int c_id,SESSION* clients)override;
	virtual void send_login_info_packet() override;
	virtual void add_session_packet(int c_id, SESSION* clients) override;
	virtual void send_remove_session_packet(int c_id) override;
	void send_obstacle_pos_packet( Obstacle* _obstacle);
	void send_login_fail_packet();
	void send_stat_changel_packet();
	void send_player_attack_packet(int c_id, short _attack_type);
	void send_monster_hp_packet(int c_id, int hp);
	void send_chat_packet(int c_id, const char* mess);
};


