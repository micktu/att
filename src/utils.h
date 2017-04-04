#pragma once

#include "stdafx.h"

#define is_dir(path) (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY)

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


template<class T> std::basic_string<T> lb_to_lit(const std::basic_string<T> &in)
{
	std::basic_string<T> out(2 * in.length(), '\0');

	size_t i = 0;
	for (char c : in)
	{
		if (c != '\r' && c != '\n')
		{
			out[i++] = c;
			continue;
		}

		if (c == '\n')
		{
			out[i++] = '\\';
			out[i++] = 'n';
		}
	}

	out.resize(i);
	return out;
}

template<class T> std::basic_string<T> lit_to_lb(const std::basic_string<T> &in)
{
	std::basic_string<T> out(in.length(), '\0');

	size_t i = 0;
	for (auto it = in.begin(); it != in.end(); ++it)
	{
		if (*it != '\\' || *(it + 1) != 'n')
		{
			out[i++] = *it;
			continue;
		}

		out[i++] = '\r';
		out[i++] = '\n';
		it++;
	}

	return out;
}

template<class T> std::basic_string<T> find_filename_suffix(const std::basic_string<T> &name, int len)
{
	std::basic_string<T>::const_iterator startPos = name.end();
	std::basic_string<T>::const_iterator dotPos = name.end();
	std::basic_string<T>::const_iterator usPos = name.end();

	// Find the starting position at the last slash
	for (auto it = name.begin(); it != name.end(); ++it)
	{
		if (*it == '\\') startPos = it;
	}
	if (startPos == name.end()) return std::basic_string<T>();

	// Find the last dot
	for (auto it = startPos + 1; it != name.end(); ++it)
	{
		if (*it == '.')
		{
			if (dotPos != name.end()) startPos = dotPos;
			dotPos = it;
		}
	}
	if (dotPos == name.end()) return std::basic_string<T>();

	// Find the last underscore
	for (auto it = startPos + 1; it != name.end(); ++it)
	{
		if (*it == '_') usPos = it;
	}

	if (usPos != name.end())
	{
		wc << L"";
	}

	if (usPos == name.end() || dotPos - usPos - 1 != len) return std::basic_string<T>();

	return std::basic_string<T>(usPos + 1, dotPos);
}
