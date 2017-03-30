#pragma once

#include "targetver.h"

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <wchar.h>

struct IUnknown; // Clang hack
#include <windows.h>


#define FORCEINLINE __forceinline
#define wcout std::wcout

using namespace std::string_literals;
using str_t = std::wstring;
using str_vector_t = std::vector<str_t>;
