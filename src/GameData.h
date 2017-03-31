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
	static const ext_vector_t DAT_EXTENSIONS;
	static const ext_vector_t TEXT_EXTENSIONS;

	str_t BasePath;
	std::vector<DatFile*> DatFiles;
	std::vector<GameFile*> GameFiles;

public:

private:

public:
	GameData() = default;
	GameData(str_t &path);

	bool Read(str_t filter);

	FORCEINLINE void GameData::ProcessFile(const str_t &path, const str_t &filename, const str_t &filter);
	bool CheckExtension(const str_t &filename, const ext_vector_t & list) const;
	FORCEINLINE bool IsDatFile(const str_t &filename) const;
	FORCEINLINE bool IsTextFile(const str_t &filename) const;
	FORCEINLINE bool IsRelevantFile(const str_t &filename, str_t filter) const;

	FORCEINLINE GameFile* operator[](int index) const { &GameFiles[index]; }

	auto begin() { return GameFiles.begin(); }
	auto end() { return GameFiles.end(); }
};