#pragma once
#include <iostream>
#include <array>
#include <WS2tcpip.h>//acepptex
#include <MSWSock.h>
#include<thread>
#include<vector>
#include <unordered_set>
#include<mutex>
#include<chrono>
#include "../../protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;
using namespace chrono;

constexpr int MAX_USER = 10000;
constexpr int MAX_NPC = 20000;

