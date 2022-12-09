#pragma once
#include "Session.h"
#include "Obstacle.h"
class Player : public SESSION
{
	
public:
	virtual void do_recv() override;
	virtual void do_send(void* packet) override;
	virtual void send_move_packet(int c_id,SESSION* clients,unsigned move_time)override;
	virtual void send_login_info_packet() override;
	virtual void add_session_packet(int c_id, SESSION* clients) override;
	virtual void send_remove_session_packet(int c_id) override;
	void send_obstacle_pos_packet( Obstacle* _obstacle);
};


