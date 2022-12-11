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
#include<string>
#include<atomic>
#include <concurrent_priority_queue.h>
#include "../../protocol.h"

extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")

using namespace std;
using namespace chrono;

//#define STRESS_TEST 0;
