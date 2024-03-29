#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <queue>
#include <chrono>
#include <unordered_map>
using namespace std;
using namespace chrono;
#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include "../../protocol.h"

sf::TcpSocket socket;

constexpr auto SCREEN_WIDTH = 20;
constexpr auto SCREEN_HEIGHT = 20;

constexpr auto TILE_WIDTH = 45;
constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH + 10;


int g_left_x;
int g_top_y;
int g_myid;
sf::RenderWindow* g_window;
sf::Font g_font;
void send_packet(void* packet);
char c_buf[6][64];
int c_enter=0;

class OBJECT {

protected:
	
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Sprite m_sprite_attack;
	sf::Sprite m_sprite_attack_2;
	sf::Text m_name;
	sf::Text m_chat;
	sf::Text m_hp_text;
	sf::Clock clock;
	sf::Clock attack_clock;
	
	sf::IntRect rectSprite;
	sf::IntRect rectSprite_attack;
	sf::IntRect rectSprite_attack_2;
	chrono::system_clock::time_point m_mess_end_time;
	
public:
	bool _is_attack;
	bool _is_buff;
	int rectSprite_left;
	int id;
	int m_x, m_y;
	char name[NAME_SIZE];
	int hp, max_hp;
	int exp, max_exp;
	int level;
	short dir;
	int power;
	short attack_type;
	chrono::system_clock::time_point buff_time;
	chrono::system_clock::time_point move_time;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		rectSprite = sf::IntRect(x, y, x2, y2);
		m_sprite.setTextureRect(rectSprite);
		m_hp_text.setFont(g_font);
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
	}

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2, sf::Sprite& a_t, sf::Sprite& a_t2) {
		m_showing = false;
		m_sprite.setTexture(t);
		rectSprite = sf::IntRect(x, y, x2, y2);
		m_sprite.setTextureRect(rectSprite);
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
		m_hp_text.setFont(g_font);
		m_sprite_attack = a_t;
		m_sprite_attack.setScale(0.5f, 0.5f);
		m_sprite_attack_2 = a_t2;
		m_sprite_attack_2.setScale(1.0f, 0.6f);
		rectSprite_attack = sf::IntRect(20, 20, 190, 170);
		rectSprite_attack_2 = sf::IntRect(0, 15, 45, 110);
		_is_attack = false;
		_is_buff = false;
	}

	OBJECT() {
		m_showing = false;
	}
	bool do_buff() {

		if (!_is_buff) {
			_is_buff = true;
			buff_time = chrono::system_clock::now();
			return true;
		}
		return false;
	}
	void show()
	{
		m_showing = true;
	}


	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}

	virtual void set_rotate(int _dir) {
		dir = _dir;
		switch (dir)
		{
		case UP:
			rectSprite.top = 15;
			break;
		case  LEFT:
			rectSprite.top = 85;
			break;
		case RIGHT:
			rectSprite.top = 155;
			break;
		case DOWN:
			rectSprite.top = 225;
			break;
		}

	}

	virtual void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * TILE_WIDTH + 8;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 8;
		m_sprite.setPosition(rx, ry);
		if (clock.getElapsedTime().asSeconds() > 0.3f)
		{
			if (rectSprite.left == 20 + 95 * 3)
				rectSprite.left = 20;
			else
				rectSprite.left += 95;

			m_sprite.setTextureRect(rectSprite);
			clock.restart();
		}


		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx + 32 - size.width / 2, ry - size.width / 2 + 10);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx + 32 - size.width / 2, ry - size.width / 2 + 10);
			g_window->draw(m_chat);
		}
	}

	void draw_not_name() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * TILE_WIDTH + 8;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);

	}

	void set_name(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		if (id < MAX_USER) m_name.setFillColor(sf::Color(255, 255, 255));
		else m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
		m_name.scale(0.9, 0.9);
	}

	void set_name_npc(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		if (id < MAX_USER) m_name.setFillColor(sf::Color(255, 255, 255));
		else m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
		m_name.scale(0.9, 0.9);
	}

	void set_chat(const char str[]) {
		m_chat.setFont(g_font);
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}
	void set_hp() {
		sf::RectangleShape rectangle(sf::Vector2f((hp / level*10), 30.0f));
		rectangle.setFillColor(sf::Color(255, 0, 0));
		float rx = (m_x - g_left_x) * TILE_WIDTH + 8;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 8;
		rectangle.setPosition(rx, ry-20);
		g_window->draw(rectangle);
		char buf[100];
		sprintf_s(buf, "%d/%d", hp, level * 10);
		m_hp_text.setPosition(rx, ry - 20);
		//m_hp_text.scale(0.8f, 0.8f);
		m_hp_text.setString(buf);
		g_window->draw(m_hp_text);
	}


	void attack() {
		float rx = (m_x - g_left_x) * TILE_WIDTH + 8;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 8;
		if (attack_type == 0) {
			switch (dir)
			{
			case LEFT:
				m_sprite_attack.setPosition(rx - TILE_WIDTH, ry);
				break;
			case RIGHT:
				m_sprite_attack.setPosition(rx + TILE_WIDTH, ry);
				break;
			case UP:
				m_sprite_attack.setPosition(rx, ry + TILE_WIDTH);
				break;
			case DOWN:
				m_sprite_attack.setPosition(rx, ry - TILE_WIDTH);
				break;
			}

			if (attack_clock.getElapsedTime().asSeconds() > 0.1f)
			{
				if (rectSprite_attack.left == 20 + 190 * 4) {
					rectSprite_attack.left = 20;
					_is_attack = false;
				}
				else
					rectSprite_attack.left += 190;

				m_sprite_attack.setTextureRect(rectSprite_attack);
				attack_clock.restart();
			}

			g_window->draw(m_sprite_attack);
		}
		else {
			for (int i = -2; i < 3; i++) {
				for (int j = -2; j < 3; j++) {
					m_sprite_attack_2.setPosition(rx + TILE_WIDTH * i, ry + TILE_WIDTH * j-7);

					if (attack_clock.getElapsedTime().asSeconds() > 0.1f)
					{
						if (rectSprite_attack_2.top == 15 + 118 * 3) {
							rectSprite_attack_2.top = 15;
							_is_attack = false;
						}
						else
							rectSprite_attack_2.top += 118;

						m_sprite_attack_2.setTextureRect(rectSprite_attack_2);
						attack_clock.restart();
					}

					g_window->draw(m_sprite_attack_2);
				}
			}
			
		}
	}
};

class Monster :public OBJECT {
	float sprite_gap;
public:
	Monster(sf::Texture& t, int x, int y, int x2, int y2,float _sprite_gap) :OBJECT(t, x, y, x2, y2), sprite_gap(_sprite_gap) {};
	~Monster() {};
	virtual void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * TILE_WIDTH + 8;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 8;
		m_sprite.setPosition(rx, ry);
		if (clock.getElapsedTime().asSeconds() > 0.3f)
		{
			if (rectSprite.left == sprite_gap * 3)
				rectSprite.left = 0;
			else
				rectSprite.left += sprite_gap;

			m_sprite.setTextureRect(rectSprite);
			clock.restart();
		}

		
		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx + 32 - size.width / 2, ry - size.width / 2 + 50);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx + 32 - size.width / 2, ry - size.width / 2 + 50);
			g_window->draw(m_chat);
		}
		set_hp();
	}

	virtual void set_rotate(int _dir) {
		dir = _dir;
		switch (dir)
		{
		case UP:
			break;
		case  LEFT:
			break;
		case RIGHT:
			break;
		case DOWN:

			break;
		}

	}

};


OBJECT avatar;
unordered_map <int, OBJECT*> players;

OBJECT white_tile;
OBJECT black_tile;
OBJECT skill_1;
OBJECT skill_2;
OBJECT skill_3;
OBJECT trees[MAX_OBSTACLE];

sf::Texture* board;
sf::Texture* pieces;
sf::Texture* rock;
sf::Texture* bluesnail;
sf::Texture* Mushroom;
sf::Texture* attack_t;
sf::Texture* attack_t_2;
sf::Texture* skill;
sf::Sprite* m_sprite;
sf::Sprite* m_sprite_2;
void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	rock = new sf::Texture;
	bluesnail = new sf::Texture;
	Mushroom = new sf::Texture;
	attack_t = new sf::Texture;
	attack_t_2 = new sf::Texture;
	m_sprite = new sf::Sprite;
	m_sprite_2 = new sf::Sprite;
	skill = new sf::Texture;
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("player.png");
	rock->loadFromFile("rock.png");
	bluesnail->loadFromFile("Monster1.png");
	Mushroom->loadFromFile("Monster2.png");
	attack_t->loadFromFile("attack.png");
	attack_t_2->loadFromFile("attack2.png");
	skill->loadFromFile("skill.png");
	m_sprite->setTexture(*attack_t);
	m_sprite->setTextureRect(sf::IntRect(420, 270, 30, 26));
	m_sprite_2->setTexture(*attack_t_2);
	m_sprite_2->setTextureRect(sf::IntRect(0, 15, 45, 110));
	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}
	white_tile = OBJECT{ *board, 4, 4, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 70, 4, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 20,20, TILE_WIDTH, TILE_WIDTH + 5, *m_sprite, *m_sprite_2 };
	skill_1 = OBJECT{ *skill, 60, 825, 55, 55 };
	skill_2 = OBJECT{ *skill, 480,825, 55, 55 };
	skill_3 = OBJECT{ *skill, 60, 765, 55, 55 };

	for (auto& tr : trees) {
		tr = OBJECT{ *rock, 0, 0, TILE_WIDTH, TILE_WIDTH };
	}

}

void client_finish()
{
	players.clear();


	delete board;
	delete pieces;
	delete rock;
	delete bluesnail;
	delete Mushroom;
	delete attack_t;
	delete attack_t_2;
	delete m_sprite;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = packet->id;
		avatar.id = g_myid;
		//cout << packet->name << endl;
		avatar.move(packet->x, packet->y);
		g_left_x = packet->x - SCREEN_WIDTH / 2;
		g_top_y = packet->y - SCREEN_WIDTH / 2;
		avatar.set_name(packet->name);
		avatar.exp = packet->exp;
		avatar.hp = packet->hp;
		avatar.level = packet->level;
		avatar.power = packet-> power;
		avatar.show();
	}
	break;
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* my_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);

			g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			g_top_y = my_packet->y - SCREEN_WIDTH / 2;
			avatar.show();
		}
		else if (id < MAX_USER) {
			players[id] = new OBJECT{ *pieces, 20,20, TILE_WIDTH, TILE_WIDTH + 5 , *m_sprite,*m_sprite_2 };
			players[id]->id = id;
			players[id]->hp = my_packet->hp;
			players[id]->level = my_packet->level;
			players[id]->move(my_packet->x, my_packet->y);
			char str3[128];
			char str1[] = " ";

			sprintf_s(str3, "LV%d", players[id]->level);
			strcat_s(str3, str1);
			strcat_s(str3, my_packet->name);
			players[id]->set_name(str3);
			players[id]->show();
		}
		else {//NPC

			if(strcmp(my_packet->name,"Horny Mushroom") == 0)
				players[id] = new Monster{ *Mushroom, 0,55, 58, 55, 58};
			else if (strcmp(my_packet->name, "bluesnail") == 0)
				players[id] = new Monster{ *bluesnail, 0,40, TILE_WIDTH, TILE_WIDTH ,50};

			players[id]->id = id;
			players[id]->hp = my_packet->hp;
			players[id]->level = my_packet->level;
			players[id]->move(my_packet->x, my_packet->y);
			char str3[128];
			char str1[] = " ";

			sprintf_s(str3, "LV%d", players[id]->level);
			strcat_s(str3, str1);
			strcat_s(str3, my_packet->name);
			players[id]->set_name(str3);
			players[id]->show();
		}
		break;
	}
	case SC_MOVE_OBJECT:
	{
	

		SC_MOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			avatar.set_rotate(my_packet->dir);
			g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			g_top_y = my_packet->y - SCREEN_WIDTH / 2;
		}
		else {
			players[other_id]->move(my_packet->x, my_packet->y);
			players[other_id]->set_rotate(my_packet->dir);
		}
		break;
	}

	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else {
			players.erase(other_id);
		}
		break;
	}
	case SC_CHAT:
	{
		SC_CHAT_PACKET* my_packet = reinterpret_cast<SC_CHAT_PACKET*>(ptr);
		int other_id = my_packet->id;
		

		for (int i = 3; i >= 0; --i) {
			strcpy_s(c_buf[i + 1], c_buf[i]);
		}

		strcpy_s(c_buf[0], my_packet->mess);
		if(c_enter < 6)
		c_enter++;

		break;
	}
	case SC_OBSTACLE:
	{
		SC_OBSTACLE_PACKET* my_packet = reinterpret_cast<SC_OBSTACLE_PACKET*>(ptr);
		trees[my_packet->id].m_x = my_packet->x;
		trees[my_packet->id].m_y = my_packet->y;
		trees[my_packet->id].show();
		break;
	}
	case SC_STAT_CHANGE:
	{
		SC_STAT_CHANGEL_PACKET* my_packet = reinterpret_cast<SC_STAT_CHANGEL_PACKET*>(ptr);
		
			avatar.hp = my_packet->hp;
			avatar.max_hp = my_packet->max_hp;
			avatar.exp = my_packet->exp;
			avatar.level = my_packet->level;
			avatar.power = my_packet->power;

		break;
	}
	case SC_PLAYERATTACK:
	{
		SC_PLAYERATTACK_PACKET* my_packet = reinterpret_cast<SC_PLAYERATTACK_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar._is_attack = true;
		}
		else {
			players[other_id]->attack_type = my_packet->attack_type;
			players[other_id]->_is_attack = true;
			
		}
		break;
	}
	case SC_MONSTERHP:
	{
		SC_MONSTER_HP_PACKET* my_packet = reinterpret_cast<SC_MONSTER_HP_PACKET*>(ptr);
		players[my_packet->id]->hp = my_packet->hp;
		

		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);//논블로킹 이니까 RECV했을때 데이터가 없으면 끝남
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result == sf::Socket::Disconnected) {
		CS_LOGOUT_PACKET p;
		p.size = sizeof(p);
		p.type = CS_LOGOUT;
		send_packet(&p);
		wcout << L"Disconnected\n";
		exit(-1);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if (((tile_x + tile_y) % 6) < 3) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}

	for (auto& tr : trees) {
		tr.draw_not_name();
	}

	avatar.draw();
	
	
	for (auto& pl : players) {
		pl.second->draw();
		
		if (pl.second->_is_attack == true) { pl.second->attack(); }
	}
	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setPosition(WINDOW_WIDTH - 200, WINDOW_HEIGHT - 50);
	text.setString(buf);
	g_window->draw(text);
	sf::Text text_player_info;
	text_player_info.setFont(g_font);
	buf[0] = '\0';
	sprintf_s(buf, " Level %d", avatar.level);
	text_player_info.setFillColor(sf::Color(0, 255, 255));
	text_player_info.setString(buf);
	g_window->draw(text_player_info);
	buf[0] = '\0';
	sprintf_s(buf, "\n HP %d / %d", avatar.hp, avatar.level * 100);
	text_player_info.setFillColor(sf::Color(255, 255, 255));
	text_player_info.setString(buf);

	sf::RectangleShape rectangle(sf::Vector2f(250.0f * ((float)avatar.hp / ((float)avatar.level * 100)), 30.0f));
	rectangle.setFillColor(sf::Color(255, 0, 0));
	rectangle.setPosition(10, 40);
	g_window->draw(rectangle);

	sf::RectangleShape rectangle_e(sf::Vector2f(250.0f*(float)avatar.exp / ((float)avatar.level * 100), 30.0f));
	rectangle_e.setFillColor(sf::Color(125, 125, 0));
	rectangle_e.setPosition(10, 110);
	g_window->draw(rectangle_e);

	g_window->draw(text_player_info);
	buf[0] = '\0';
	sprintf_s(buf, "\n\n POWER %d ", avatar.power);
	text_player_info.setFillColor(sf::Color(0, 255, 0));
	text_player_info.setString(buf);

	g_window->draw(text_player_info);
	buf[0] = '\0';
	sprintf_s(buf, "\n\n\n EXP %d / %d", avatar.exp, avatar.level * 100);
	text_player_info.setFillColor(sf::Color(255, 255, 255));
	text_player_info.setString(buf);

	g_window->draw(text_player_info);
	
	float rx = (avatar.m_x - g_left_x) * 55 + 8;
	float ry = (avatar.m_y - g_top_y) * 55 + 250;
	skill_1.a_move(rx, ry );
	skill_1.a_draw();
	
	skill_2.a_move(rx+100, ry);
	skill_2.a_draw();
	
	skill_3.a_move(rx+200, ry);
	skill_3.a_draw();

	if (avatar._is_attack == true) {
		for (int i = 0; i < 2; i++) {
			float rx = (avatar.m_x - g_left_x) * 55 + 8 + i*100;
			float ry = (avatar.m_y - g_top_y) * 55 + 250;
			sf::RectangleShape rectangle(sf::Vector2f(50.0f, 50.0f));
			rectangle.setFillColor(sf::Color(128, 128, 128, 100));
			rectangle.setPosition(rx, ry);
		
			g_window->draw(rectangle);
		}
		avatar.attack();
	}
	if (avatar._is_buff == true) {
		if (avatar.buff_time + 10s < chrono::system_clock::now()) {
			avatar._is_buff = false;
		}
		else {
			float rx = (avatar.m_x - g_left_x) * 55 + 8 + 200;
			float ry = (avatar.m_y - g_top_y) * 55 + 250;
			sf::RectangleShape rectangle(sf::Vector2f(50.0f, 50.0f));
			rectangle.setFillColor(sf::Color(128, 128, 128, 100));
			rectangle.setPosition(rx, ry);
			g_window->draw(rectangle);
		}
	}

	sf::RectangleShape rectangle_t(sf::Vector2f(450.0f, 300.0f));
	rectangle_t.setFillColor(sf::Color(128, 128, 128, 100));
	rectangle_t.setPosition(0, WINDOW_HEIGHT - 300);
	g_window->draw(rectangle_t);

	text.setScale(0.7f, 0.7f);
	for (int i = 0; i < c_enter; i++) {
		text.setPosition(0, WINDOW_HEIGHT - 100 - 30*i);
		text.setString(c_buf[i]);
	
		g_window->draw(text);
	}


}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	socket.send(packet, p[0], sent);
}

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = socket.connect("127.0.0.1", PORT_NUM);
	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;


	char arr[16];
	cout << "Enter ID : ";
	cin.getline(arr, BUF_SIZE);
	strcpy_s(p.name, arr);
	send_packet(&p);

	client_initialize();


	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int direction = -1;
				bool attack = false;
				if (!avatar._is_attack) {
					switch (event.key.code) {
					case sf::Keyboard::Left:
						if (avatar.move_time + 0.1s < chrono::system_clock::now()) {
							avatar.move_time = chrono::system_clock::now();
							direction = 2;
						}

						break;
					case sf::Keyboard::Right:
						if (avatar.move_time + 0.1s < chrono::system_clock::now()) {
							avatar.move_time = chrono::system_clock::now();
							direction = 3;
						}
						break;
					case sf::Keyboard::Up:
						if (avatar.move_time + 0.1s < chrono::system_clock::now()) {
							avatar.move_time = chrono::system_clock::now();
							direction = 0;
						}
						break;
					case sf::Keyboard::Down:
						if (avatar.move_time + 0.1s < chrono::system_clock::now()) {
							avatar.move_time = chrono::system_clock::now();
							direction = 1;
						}
						break;
					case sf::Keyboard::Escape:
						window.close();
						break;
					case sf::Keyboard::A:
						attack = true;
						avatar._is_attack = true;
						avatar.attack_type = 0; //방향공격
						break;

					case sf::Keyboard::S:
						attack = true;
						avatar._is_attack = true;
						avatar.attack_type = 1; //범위공격
						break;

					case sf::Keyboard::D: //버프 
						if (avatar.do_buff()) {
							CS_BUFF_PACKET p;
							p.size = sizeof(p);
							p.type = CS_BUFF;
							send_packet(&p);
						}
						break;
					}

				}
				if (-1 != direction) {
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = direction;
					send_packet(&p);
				}
				if (attack == true)
				{
					CS_ATTACK_PACKET p;
					p.size = sizeof(p);
					p.type = CS_ATTACK;
					p.attack_type = avatar.attack_type;
					cout <<"attack_type = " << avatar.attack_type << endl;
					send_packet(&p);
					attack = false;
				}
			}
		}

		window.clear();
		client_main();
		window.display();
	}

	client_finish();

	return 0;
}