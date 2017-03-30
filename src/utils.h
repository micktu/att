#pragma once

#include "stdafx.h"

#define is_dir(path) (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY)
#define wstr_as_long(s) (*(uint64_t*)s)
#define ext_equals(filename, ext) (wstr_as_long(filename.c_str() + filename.length() - 4) != wstr_as_long(ext.c_str()))

str_t path_strip_filename(str_t filename);
void split_path(const str_t & path, str_t & name, str_t & ext);
str_t strip_slash(const str_t &path);
str_t add_slash(const str_t &path);
str_vector_t find_files_recursive(str_t path, int pathSize);
str_vector_t find_files(str_t path);
void create_dir_recursive(str_t path);
str_t cstr_to_wstr(const char * cstr);
std::ifstream::pos_type get_file_size(str_t filename);
