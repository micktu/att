#pragma once

#include "stdafx.h"

#define is_dir(path) (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY)
#define wcs_as_long(str) (*reinterpret_cast<const uint64_t*>(str))

bool ext_equals(const str_t &filename, const wchar_t *ext);
str_t add_slash(const str_t &path);
str_t strip_slash(const str_t &path);
str_t cstr_to_wstr(const char * cstr);
str_t path_strip_filename(str_t filename);
str_t path_normalize(str_t path);
void split_path(const str_t &str, str_t &path, str_t &name, str_t &ext);
void create_dir_recursive(str_t path);
size_t get_file_size(const str_t& filename);
str_vector_t find_files_recursive(str_t path, int pathSize);
str_vector_t find_files(str_t path);
