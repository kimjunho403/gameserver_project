#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <unordered_map>
using namespace std;

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

class OBJECT {

protected:
	
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Sprite m_sprite_attack;
	sf::Text m_name;
	sf::Text m_chat;
	sf::Text m_hp_text;
	sf::Clock clock;
	sf::Clock attack_clock;
	sf::IntRect rectSprite;
	sf::IntRect rectSprite_attack;
	chrono::system_clock::time_point m_mess_end_time;
public:
	bool _is_attack;
	int rectSprite_left;
	int id;
	int m_x, m_y;
	char name[NAME_SIZE];
	int hp, max_hp;
	int exp, max_exp;
	int level;
	short dir;
	int power;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		rectSprite = sf::IntRect(x, y, x2, y2);
		m_sprite.setTextureRect(rectSprite);
		m_hp_text.setFont(g_font);
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
	}

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2, sf::Sprite& a_t) {
		m_showing = false;
		m_sprite.setTexture(t);
		rectSprite = sf::IntRect(x, y, x2, y2);
		m_sprite.setTextureRect(rectSprite);
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
		m_hp_text.setFont(g_font);
		m_sprite_attack = a_t;
		m_sprite_attack.setScale(0.5f, 0.5f);
		rectSprite_attack = sf::IntRect(20, 20, 190, 170);
	}

	OBJECT() {
		m_showing = false;
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


		//	attack_clock.restart();

		g_window->draw(m_sprite_attack);

	}
};

class Monster :public OBJECT {
public:
	Monster(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT(t, x, y, x2, y2) {};
	~Monster() {};
	virtual void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * TILE_WIDTH + 8;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 8;
		m_sprite.setPosition(rx, ry);
		if (clock.getElapsedTime().asSeconds() > 0.3f)
		{
			if (rectSprite.left == 50 * 3)
				rectSprite.left = 0;
			else
				rectSprite.left += 50;

			m_sprite.setTextureRect(rectSprite);
			clock.restart();
		}

		
		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx + 32 - size.width / 2, ry - size.width / 2 + 30);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx + 32 - size.width / 2, ry - size.width / 2 + 30);
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
OBJECT trees[MAX_OBSTACLE];

sf::Texture* board;
sf::Texture* pieces;
sf::Texture* rock;
sf::Texture* bluesnail;
sf::Texture* attack_t;
sf::Sprite* m_sprite;
void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	rock = new sf::Texture;
	bluesnail = new sf::Texture;
	attack_t = new sf::Texture;
	m_sprite = new sf::Sprite;

	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("player.png");
	rock->loadFromFile("rock.png");
	bluesnail->loadFromFile("Monster1.png");
	attack_t->loadFromFile("attack.png");


	m_sprite->setTexture(*attack_t);
	//m_sprite->setTextureRect(sf::IntRect(420, 140, 30, 26));
	m_sprite->setTextureRect(sf::IntRect(420, 270, 30, 26));

	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}
	white_tile = OBJECT{ *board, 4, 4, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 70, 4, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 20,20, TILE_WIDTH, TILE_WIDTH + 5, *m_sprite };



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
	delete attack_t;
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
			players[id] = new OBJECT{ *pieces, 20,20, TILE_WIDTH, TILE_WIDTH + 5 , *m_sprite };
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
			players[id] = new Monster{ *bluesnail, 0,40, TILE_WIDTH, TILE_WIDTH };
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
		if (other_id == g_myid) {
			avatar.set_chat(my_packet->mess);
		}
		else {
			players[other_id]->set_chat(my_packet->mess);
		}

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
	case SC_PLAYERINFO:
	{
		SC_PLAYERINFO_PACKET* my_packet = reinterpret_cast<SC_PLAYERINFO_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hp = my_packet->hp;
			avatar.exp = my_packet->exp;
			avatar.level = my_packet->level;
		}
		else {
			players[other_id]->hp = my_packet->hp;
			players[other_id]->exp = my_packet->exp;
			players[other_id]->level = my_packet->level;
		}
		

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
			players[other_id]->_is_attack = true;
		}
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
	if (avatar._is_attack == true)avatar.attack();
	
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
	sprintf_s(buf, "\n HP %d / %d", avatar.hp, avatar.hp * 10);
	text_player_info.setFillColor(sf::Color(255, 255, 255));
	text_player_info.setString(buf);
	g_window->draw(text_player_info);
	buf[0] = '\0';
	sprintf_s(buf, "\n\n POWER %d ", avatar.level *5);
	text_player_info.setFillColor(sf::Color(0, 255, 0));
	text_player_info.setString(buf);
	sf::RectangleShape rectangle(sf::Vector2f(128.0f * (10.0f / 100.0f), 30.0f));
	rectangle.setFillColor(sf::Color(255, 0, 0));
	rectangle.setPosition(10, 40);
	g_window->draw(rectangle);
	g_window->draw(text_player_info);


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
				switch (event.key.code) {
				case sf::Keyboard::Left:
					direction = 2;

					break;
				case sf::Keyboard::Right:
					direction = 3;
					break;
				case sf::Keyboard::Up:
					direction = 0;
					break;
				case sf::Keyboard::Down:
					direction = 1;
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::A:
					attack = true;
					avatar._is_attack = true;
					break;
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