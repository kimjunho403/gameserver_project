#include "global.h"
#include "Session.h"

SESSION::SESSION() : _id(-1), _state(ST_FREE), _socket(0), _x(0), _y(0), _prev_remain(0), last_movetime(0)
{
	_name[0] = 0;
}

SESSION::~SESSION()
{

}