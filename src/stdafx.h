#pragma once

#include "targetver.h"

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <wchar.h>

struct IUnknown; // Clang hack
#include <windows.h>

#define FORCEINLINE __forceinline
#define wc (std::wcout)

using namespace std::string_literals;

using wcs_vector_t = std::vector<const wchar_t*>;
using char_vector_t = std::vector<char>;

using str_t = std::string;
using str_vec_t = std::vector<str_t>;
using str_map_t = std::map<str_t, str_t>;

using wstr_t = std::wstring;
using wstr_vec_t = std::vector<wstr_t>;
using wstr_map_t = std::map<wstr_t, wstr_t>;

using cmd_callback_t = void(*)(int&, wchar_t **&);
using cmd_map_t = std::map<wstr_t, cmd_callback_t>;
