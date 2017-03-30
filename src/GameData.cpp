#include "GameData.h"
#include "utils.h"

constexpr wchar_t GameData::TEXT_EXTENSIONS[][5];

GameData::GameData(str_t &path)
{
	Read(path);
}

bool GameData::Read(str_t path)
{
	uint32_t attributes = GetFileAttributes(path.c_str());
	if (INVALID_FILE_ATTRIBUTES == attributes) return false;

	BasePath = path;

	DatFiles.clear();
	DatFiles.reserve(0x1000);
	GameFiles.clear();
	GameFiles.reserve(0x1000);

	if ((FILE_ATTRIBUTE_DIRECTORY & attributes) == 0)
	{
		if (!DatFile::CheckFile(path)) return false;
		DatFiles.emplace_back(new DatFile(path));
		return true;
	}

	str_vector_t files = find_files(path);

	path = add_slash(path);
	for (str_t &filename : files)
	{
		if (!IsRelevantFile(filename)) continue;

		str_t fullPath = path + filename;
		GameFile* gf = new GameFile(filename, get_file_size(fullPath));
		GameFiles.emplace_back(gf);

		if (DatFile::CheckFile(fullPath))
		{
			DatFile *dat = new DatFile(fullPath);
			DatFiles.emplace_back(dat);

			str_t relPath = path_strip_filename(filename);

			gf->Dat = dat;
			gf->bIsContainer = true;
			gf->Files.reserve(dat->NumEntries());

			int i = 0;
			for (DatFileEntry &entry : *dat)
			{
				if (!IsRelevantFile(entry.Name)) continue;

				str_t relName = add_slash(path);
				gf->Files.emplace_back(relName + entry.Name, entry.Size, gf, dat, i);
				++i;
			}

			continue;
		}
	}

	return true;
}

bool GameData::IsRelevantFile(str_t &filename)
{
	return true;

	for (const str_t &ext : TEXT_EXTENSIONS)
	{
		if (ext_equals(filename, ext)) return true;
	}

	return false;
}

GameFile::GameFile(str_t filename, size_t size, const GameFile* container, const DatFile* dat, int index)
{
	RelPath = filename;
	split_path(RelPath, Filename, Extension);

	Size = size;
	Container = container;
	Dat = dat;
	Index = index;
}
