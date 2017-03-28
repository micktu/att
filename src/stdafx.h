// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

struct IUnknown; // Clang hack
#include <windows.h>

#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <wchar.h>


using namespace std::string_literals;

#define FORCEINLINE __forceinline

#define cout std::wcout
#define str_t std::wstring
#define str_vector_t std::vector<str_t>

// TODO: reference additional headers your program requires here
