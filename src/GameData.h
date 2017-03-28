#pragma once

#include "stdafx.h"

class DatFile;

class GameData
{
	struct GameFile {
		wchar_t FullPath[MAX_PATH];
		wchar_t *Filename;
		wchar_t *Extension;

		uint32_t Size;

		DatFile* Dat;
		int Index;

		size_t SizeDir() { return Filename - FullPath - 1; }
		size_t SizeName() { return Extension - Filename - 1; }

	};

	wchar_t BasePath[MAX_PATH];
	std::vector<std::wstring> PlainFileNames;
	std::vector<std::wstring> DatFileNames;
	std::vector<DatFile> DatFiles;

public:

private:

public:
	GameData();
	~GameData();

	GameData(const wchar_t* path);
	
	bool Read(const wchar_t* path);

	void ListFiles();
};