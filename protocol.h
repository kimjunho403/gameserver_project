constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 100;
constexpr int VIEW_RANGE = 15;
constexpr int MAX_USER = 10000;
//constexpr int MAX_NPC = 200000;
constexpr int MAX_NPC = 20000;
constexpr int MAX_OBSTACLE = 10000;
constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int DOWN = 0;
constexpr int UP = 1;
constexpr int LEFT = 2;
constexpr int RIGHT = 3;
// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_CHAT = 2; //채팅
constexpr char CS_ATTACK = 3;			// 4 방향 공격
constexpr char CS_TELEPORT = 4;			// RANDOM한 위치로 Teleport, Stress Test할 때 Hot Spot현상을 피하기 위해 구현 stress 테스트에서 쓰는거
constexpr char CS_LOGOUT = 5;			// 클라이언트에서 정상적으로 접속을 종료하는 패킷
constexpr char CS_BUFF = 6;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_OBJECT = 3;
constexpr char SC_REMOVE_OBJECT = 4;
constexpr char SC_MOVE_OBJECT = 5;
constexpr char SC_CHAT = 6;
constexpr char SC_LOGIN_OK = 7;
constexpr char SC_LOGIN_FAIL = 8;
constexpr char SC_STAT_CHANGE = 9;
constexpr char SC_OBSTACLE = 10;
constexpr char SC_PLAYERATTACK = 11;
constexpr char SC_MONSTERHP = 12;
#pragma pack (push, 1)

struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned	move_time;
};

struct CS_CHAT_PACKET {
	unsigned char size;
	char	type;
	char	mess[CHAT_SIZE];
};

struct CS_TELEPORT_PACKET {
	unsigned char size;
	char	type;
};

struct CS_LOGOUT_PACKET {
	unsigned char size;
	char	type;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
	short attack_type;
};
struct CS_BUFF_PACKET {
	unsigned char size;
	char	type;
};


struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	short	x, y;
	char	name[NAME_SIZE];
	int		id;
	int		hp;
	int		exp;
	int		level;
	int power;
};

struct SC_ADD_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	char	name[NAME_SIZE];
	int		hp;
	int		level;
};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
};

struct SC_MOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	short dir;
	unsigned int move_time;
};

struct SC_CHAT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	char	mess[CHAT_SIZE];
};

struct SC_LOGIN_OK_PACKET {
	unsigned char size;
	char	type;
};

struct SC_LOGIN_FAIL_PACKET {
	unsigned char size;
	char	type;

};

struct SC_STAT_CHANGEL_PACKET {
	unsigned char size;
	char	type;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
	int		power;
};

struct SC_OBSTACLE_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
};

struct SC_PLAYERATTACK_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short attack_type;
};
struct SC_MONSTER_HP_PACKET {
	unsigned char size;
	char	type;
	int		id;
	int		hp;
};
#pragma pack (pop)