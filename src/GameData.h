#pragma once

#include "stdafx.h"
#include "GameFile.h"
#include "DatFile.h"

class GameData
{
	static const ext_vector_t DAT_EXTENSIONS;
	static const ext_vector_t TEXT_EXTENSIONS;

	str_t BasePath;
	str_vector_t Filenames;
	std::vector<DatFile> DatFiles;
	std::vector<GameFile> GameFiles;

public:

private:

public:
	GameData() = default;
	GameData(str_t &path);

	bool Read(str_t filter);

	FORCEINLINE void ProcessFile(const str_t &filename, const str_t &filter);
	bool CheckExtension(const str_t &filename, const ext_vector_t & list) const;
	FORCEINLINE bool IsDatFile(const str_t &filename) const;
	FORCEINLINE bool IsTextFile(const str_t &filename) const;
	FORCEINLINE bool IsRelevantFile(const str_t &filename, str_t filter) const;

	FORCEINLINE str_t GetBasePath() { return BasePath; }
	FORCEINLINE str_vector_t& GetFilenames() { return Filenames; }
	FORCEINLINE std::vector<GameFile>& GetGameFiles() { return GameFiles; }
	FORCEINLINE std::vector<DatFile>& GetDatFiles() { return DatFiles; }

	FORCEINLINE GameFile& operator[](int index) { return GameFiles[index]; }
	auto begin() { return GameFiles.begin(); }
	auto end() { return GameFiles.end(); }
};