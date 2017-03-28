#pragma once

#include "stdafx.h"

#include "DatFile.h"

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

	str_t BasePath;
	str_vector_t PlainFileNames;
	str_vector_t DatFileNames;
	std::vector<DatFile> DatFiles;

public:

private:

public:
	GameData() = default;

	GameData(str_t &path);
	
	bool Read(str_t path);

	void ListFiles();
};