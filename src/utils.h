#pragma once

#include "stdafx.h"


void split_path(wchar_t* path, wchar_t* &name, wchar_t* &ext);
void split_path(const str_t & path, str_t & name, str_t & ext);
str_vector_t find_files_recursive(str_t path, int pathSize);
str_vector_t find_files(str_t path);
void create_dir_recursive(str_t path);
str_t cstr_to_wstr(const char * cstr);
std::ifstream::pos_type get_file_size(str_t filename);
