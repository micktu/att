#pragma once

#include "stdafx.h"

#define DAT_MAGIC '\x00TAD'

typedef uint32_t dat_offset_t, dat_size_t;

typedef char dat_name_t[MAX_PATH];
typedef char dat_ext_t[4];

struct dat_header
{
	uint32_t magic;
	uint32_t num_files;
	dat_offset_t files;
	dat_offset_t extensions;
	dat_offset_t names;
	dat_offset_t sizes;
	dat_offset_t unknown1;
	uint32_t unknown2;
};

struct DatFileEntry
{
	uint32_t Index;
	dat_ext_t Extension;
	dat_name_t Name;
	dat_size_t Size;
	dat_offset_t Offset;

	//class DatFile* Dat;
};

class DatFile
{
	wchar_t _filename[MAX_PATH];
	std::vector<DatFileEntry> _entries;
	dat_header _header;

public:

private:

public:
	static bool CheckFile(const wchar_t* path);
	
	DatFile() = default;
	DatFile(const DatFile &obj) = default;
	DatFile(const wchar_t* filename);

	bool Read(const wchar_t* filename);
	void ReadFile(const DatFileEntry* entry, char* buffer);
	void InjectFile(int index, char * buffer, uint32_t numBytes);
	const DatFileEntry* FindFile(const char* name);
	void ExtractFile(const DatFileEntry* entry, const char * outPath);
	void ExtractAll(const char* outPath);

	__forceinline const wchar_t* GetFilename() const { return _filename; }
	__forceinline size_t NumEntries() const { return _entries.size(); }
	__forceinline DatFileEntry* operator[](int index) { return &_entries[index]; }
};
