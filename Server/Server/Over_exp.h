#pragma once

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND,OP_NPC_MOVE, OP_AI_NEAR_CHECK, OP_NPC_RESPAWN, OP_PLAYER_HPUP,OP_PAYER_BUFF , OP_CHASE};

class OVER_EXP
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int _ai_target_obj;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}

	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];//패킷의 길이
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}

};