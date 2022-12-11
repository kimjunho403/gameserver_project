#pragma once
#include "Session.h"
#include "Obstacle.h"

enum EVENT_TYPE { EV_RANDOM_MOVE };

struct TIMER_EVENT {
	int obj_id;
	chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	int target_id;
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};



class Monster : public SESSION
{
public:
	atomic_bool	_is_active;		// 주위에 플레이어가 있는가?

public:
	virtual void do_recv() override;
	virtual void do_send(void* packet) override;
	virtual void send_move_packet(int c_id, SESSION* clients, unsigned move_time)override;
	virtual void send_login_info_packet() override;
	virtual void add_session_packet(int c_id, SESSION* clients) override;
	virtual void send_remove_session_packet(int c_id) override;

};


