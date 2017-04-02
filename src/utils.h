#pragma once

#include "stdafx.h"

#define is_dir(path) (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY)
#define wcs_as_long(str) (*reinterpret_cast<const uint64_t*>(str))

bool ext_equals(const wstr_t &filename, const wchar_t *ext);
wstr_t add_slash(const wstr_t &path);
wstr_t strip_slash(const wstr_t &path);
wstr_t cstr_to_wstr(const char * cstr);
wstr_t path_strip_filename(wstr_t filename);
wstr_t path_normalize(wstr_t path);
void split_path(const wstr_t &str, wstr_t &path, wstr_t &name, wstr_t &ext);
void create_dir_recursive(wstr_t path);
size_t get_file_size(const wstr_t& filename);
wstr_t utf8_to_wstr(const char * bytes);
std::string wstr_to_utf8(wstr_t str);
wstr_vec_t find_files_recursive(wstr_t path, size_t pathSize);
wstr_vec_t find_files(wstr_t path);
str_t format_loc_message(struct LocMessage &message);