#include "utils.h"
#include <Shlobj.h>


#define is_sep(p) (*p == '\\' || *p == '/')
void split_path(const wchar_t *path, const wchar_t **name, const wchar_t **ext)
{
	*name = nullptr;
	*ext = nullptr;

	if (path == nullptr) return;

	size_t len = wcslen(path);
	const wchar_t *p = path + len - 1;

	if (is_sep(p)) p--;

	while (p >= path)
	{
		if (is_sep(p))
		{
			*name = p + 1;
			break;
		}

		if (*p == '.' && *ext == nullptr && p > path && !is_sep(p - 1))
		{
			*ext = p + 1;
		}

		p--;
	}

	if (*name == nullptr) *name = path;
	if (*ext == nullptr) *ext = path + len;
}

void split_path(const str_t &path, str_t &name, str_t &ext)
{
	const wchar_t *cname, *cext;
	split_path(path.c_str(), &cname, &cext);
	name = str_t(cname);
	ext = str_t(cext);
}

str_vector_t find_files_recursive(str_t path, int pathSize = -1)
{
	if (pathSize < 0) pathSize = path.size();
	path += L"\\";
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
	}
	while (FindNextFile(hFind, &ffd) != 0);

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

void create_dir_recursive(str_t path)
{
	wchar_t fullPath[MAX_PATH];
	GetFullPathName(path.c_str(), MAX_PATH, fullPath, NULL);
	SHCreateDirectoryEx(NULL, fullPath, NULL);
}

str_t cstr_to_wstr(const char* cstr)
{
	size_t len = strlen(cstr);
	wchar_t* buf = new wchar_t[len + 1];

	size_t numChars;
	mbstowcs_s(&numChars, buf, len + 1, cstr, len);

	str_t wstr(buf);
	delete[] buf;
	return wstr;
}

std::ifstream::pos_type get_file_size(str_t filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}
