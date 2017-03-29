#pragma once

#include "stdafx.h"

#include "DatFile.h"

struct GameFile
{
	GameFile() = default;
	GameFile(str_t filename, size_t size = 0, const GameFile* Container = nullptr, const DatFile* dat = nullptr, int index = -1);

	str_t RelPath;
	str_t Filename;
	str_t Extension;

	size_t Size;

	bool bIsContainer;
	std::vector<GameFile> Files;
	const GameFile* Container;
	const DatFile* Dat;
	int Index;
};

class GameData
{
	str_t BasePath;
	std::vector<DatFile*> DatFiles;
	std::vector<GameFile*> GameFiles;

public:

private:

public:
	GameData() = default;
	GameData(str_t &path);

	bool Read(str_t path);
	bool IsRelevantFile(str_t & filename);

	FORCEINLINE std::vector<GameFile*>& GetGameFiles() { return GameFiles; }
};