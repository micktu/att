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
	std::map<wstr_t, GameFile> GameFiles;

public:

private:

public:
	GameData() = default;
	GameData(wstr_t &path);

	bool Read(wstr_t filter);

	bool CheckExtension(const wstr_t &filename, const wcs_vector_t & list) const;
	bool IsDatFile(const wstr_t &filename) const;
	bool IsTextFile(const wstr_t &filename) const;
	bool IsRelevantFile(const wstr_t &filename, wstr_t filter) const;

	const wstr_t &GetBasePath() const { return BasePath; }
	wstr_vec_t& GetFilenames() { return Filenames; }
	std::map<wstr_t, GameFile>& GetGameFiles() { return GameFiles; }
	std::vector<DatFile>& GetDatFiles() { return DatFiles; }

	GameFile& operator[](wstr_t id) { return GameFiles[id]; }
	auto begin() { return GameFiles.begin(); }
	auto end() { return GameFiles.end(); }
};