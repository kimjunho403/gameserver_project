#pragma once
#include "Session.h"
#include "Obstacle.h"





class Monster : public SESSION
{
public:
	atomic_bool	_is_active;		// 주위에 플레이어가 있는가?
	atomic_bool _is_die;
public:
	virtual void do_recv() override;
	virtual void do_send(void* packet) override;
	virtual void send_move_packet(int c_id, SESSION* clients, unsigned move_time)override;
	virtual void send_login_info_packet() override;
	virtual void add_session_packet(int c_id, SESSION* clients) override;
	virtual void send_remove_session_packet(int c_id) override;
	void chase_move(SESSION* client);

};


