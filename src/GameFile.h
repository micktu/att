#pragma once

#include "stdafx.h"

class GameData;
struct DatFileEntry;

struct GameFile
{
	GameFile() = default;
	GameFile(GameData *gf, const wstr_t &filename, size_t index, size_t size = -1, size_t datIndex = -1, size_t indexInDat = -1);

	wstr_t FullPath;
	wstr_t Path;
	wstr_t Filename;
	wstr_t Extension;

	size_t Index;
	size_t Size;
	size_t DatIndex;
	size_t IndexInDat;

	GameData *Data;

	DatFileEntry *GetDatEntry() const;
};
