#pragma once

#include "targetver.h"

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <wchar.h>

struct IUnknown; // Clang hack
#include <windows.h>


#define FORCEINLINE __forceinline
#define wcout std::wcout

using namespace std::string_literals;
using str_t = std::wstring;
using str_vector_t = std::vector<str_t>;
using cmd_t = void(*)(int&, wchar_t **&);
using cmd_map_t = std::map<str_t, cmd_t>;
using ext_vector_t = std::vector<wchar_t*>;