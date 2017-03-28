#pragma once

#include "stdafx.h"

#include <Shlobj.h>


#define is_dir(path) (GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY)
#define is_dat(f) strcmp(f + strlen(f) - 4, ".dat") == 0
#define is_sep(p) (*p == '\\' || *p == '/')

void find_file_name_ext(const wchar_t* const path, const wchar_t** const name, const wchar_t** const ext) {
	*name = nullptr;
	*ext = nullptr;

	if (path == nullptr) return;

	const wchar_t *p = path + wcslen(path) - 1;

	if (is_sep(p)) p--;

	while (p >= path)
	{
		if (is_sep(p))
		{
			*name = p + 1;
			break;
		}

		if (*p == '.' && *ext == nullptr && p > path && !is_sep(p-1))
		{
			*ext = p + 1;
		}

		p--;
	}

	if (*name == nullptr) *name = path;
}

str_vector_t find_files_recursive(str_t path, int pathSize = -1)
{
	if (pathSize < 0) pathSize = path.size();
	path.append(L"\\");
	str_t dirMask = path + L"*";

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(dirMask.c_str(), &ffd);

	str_vector_t files;
	if (INVALID_HANDLE_VALUE == hFind) return files;

	do
	{
		wchar_t* name = ffd.cFileName;
		if (name[0] == '.' && (name[1] == '.' || name[1] == 0))
		{
			continue;
		}

		str_t filename = path + name;
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			str_vector_t inner = find_files_recursive(filename, pathSize);
			files.insert(files.end(), inner.begin(), inner.end());
			continue;
		}

		files.emplace_back(filename.c_str() + pathSize + 1);
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);

	return files;
}

str_vector_t find_files(str_t path)
{
	str_vector_t files;

	wchar_t* last = &path.back();
	if (*last == '\\') *last = 0;

	bool bIsDirectory = GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
	if (!bIsDirectory) return files;

	return find_files_recursive(path);
}

void create_dir_recursive(char* path)
{
	char fullPath[MAX_PATH];
	GetFullPathNameA(path, MAX_PATH, fullPath, NULL);
	SHCreateDirectoryExA(NULL, fullPath, NULL);
}
