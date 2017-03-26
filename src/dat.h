#pragma once

#include "stdafx.h"


typedef uint32_t dat_offset, dat_size;

typedef char dat_name[256];

struct dat_header
{
	uint32_t magic;
	uint32_t num_files;
	dat_offset files;
	dat_offset extensions;
	dat_offset names;
	dat_offset sizes;
	dat_offset unknown1;
	uint32_t unknown2;
};

union dat_extension
{
	char str[4];
	int num;
};

struct dat_file
{
	FILE* file;
	dat_header header;
	int num_files;
	dat_offset* files;
	dat_extension* extensions;
	dat_name* names;
	dat_size* sizes;
};

class DatFile
{
	int _numFiles;
	std::fstream _file;

	dat_header _header;
	dat_offset* _files;
	dat_extension* _extensions;
	dat_name* _names;
	dat_size* _sizes;

public:

private:

public:
	DatFile(const char* filename);
	~DatFile();

	__forceinline void Seek(uint32_t index) { _file.seekg(_files[index]); }

	__forceinline int NumFiles() { return _numFiles; }
	__forceinline std::fstream* GetFile() { return &_file; }

	__forceinline dat_extension* GetExtensions() const { return _extensions; }
	__forceinline dat_name* GetNames() const { return _names; }
	__forceinline dat_offset* GetFiles() const { return _files; }
};
