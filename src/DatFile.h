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

struct DatFileEntry
{
	dat_extension Extension;
	dat_name Name;
	dat_size Size;
	dat_offset Offset;
};

class DatFile
{
	std::fstream _file;
	int _numEntries;
	DatFileEntry* _entries;
	dat_header _header;

public:

private:

public:
	DatFile(const char* filename);
	~DatFile();

	void ReadFile(int index, char* buffer);
	void InjectFile(int index, char * buffer, uint32_t numBytes);
	int FindFile(const char* name);
	void ExtractFile(int index, const char* outPath);
	void ExtractAll(const char* outPath);
	void PrintFiles();

	__forceinline int NumEntries() const { return _numEntries; }
	__forceinline DatFileEntry* operator[](int index) { return _entries + index; }
};
