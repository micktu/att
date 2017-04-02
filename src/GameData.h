#pragma once

#include "stdafx.h"
#include "GameFile.h"
#include "DatFile.h"

class GameData
{
	static const wcs_vector_t DAT_EXTENSIONS;
	static const wcs_vector_t TEXT_EXTENSIONS;

	wstr_t BasePath;
	wstr_vec_t Filenames;
	std::vector<DatFile> DatFiles;
	std::vector<GameFile> GameFiles;

public:

private:

public:
	GameData() = default;
	GameData(wstr_t &path);

	bool Read(wstr_t filter);

	FORCEINLINE void ProcessFile(const wstr_t &filename, const wstr_t &filter);
	bool CheckExtension(const wstr_t &filename, const wcs_vector_t & list) const;
	FORCEINLINE bool IsDatFile(const wstr_t &filename) const;
	FORCEINLINE bool IsTextFile(const wstr_t &filename) const;
	FORCEINLINE bool IsRelevantFile(const wstr_t &filename, wstr_t filter) const;

	FORCEINLINE wstr_t GetBasePath() { return BasePath; }
	FORCEINLINE wstr_vec_t& GetFilenames() { return Filenames; }
	FORCEINLINE std::vector<GameFile>& GetGameFiles() { return GameFiles; }
	FORCEINLINE std::vector<DatFile>& GetDatFiles() { return DatFiles; }

	FORCEINLINE GameFile& operator[](int index) { return GameFiles[index]; }
	auto begin() { return GameFiles.begin(); }
	auto end() { return GameFiles.end(); }
};