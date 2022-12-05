#pragma once
#include "Session.h"

class Player : public SESSION
{
public:
	virtual void do_recv() override;
	virtual void do_send(void* packet) override;
	virtual void send_move_packet(int c_id,SESSION*)override;
	virtual void send_login_info_packet() override;
};


