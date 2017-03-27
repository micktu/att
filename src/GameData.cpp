#include "GameData.h"

GameData::GameData()
{
}

GameData::~GameData()
{
}

GameData::GameData(const wchar_t * path)
{
	Read(path);
}

bool GameData::Read(const wchar_t* path)
{
	// determine if path is file or dir
	// when dir, do recursion
	// collect list of dat files
	// collect list of contained files pointing to their dat files

	if (DatFile::CheckFile(path))
	{
		DatFiles.emplace_back(path);
		return true;
	}

	return true;
}