#include "GameData.h"

#include "DatFile.h"

#include "utils.h"

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

	wcscpy_s(BasePath, path);
	PlainFileNames.clear();
	DatFileNames.clear();
	DatFiles.clear();

	if (DatFile::CheckFile(path))
	{
		DatFiles.emplace_back(path);
		return true;
	}

	if (is_dir(path))
	{ 
		path_vector_t files = find_files(path);

		wchar_t fullPath[MAX_PATH];
		for (std::wstring &filename : files)
		{
			wsprintf(fullPath, L"%s\\%s", path, filename.c_str());

			if (DatFile::CheckFile(fullPath))
			{
				DatFileNames.push_back(filename);
				DatFiles.emplace_back(fullPath);
				continue;
			}

			PlainFileNames.push_back(filename);
		}

		return true;
	}


	return true;
}

void GameData::ListFiles()
{
	int i = 0;
	std::vector<DatFile>::iterator it;
	for (it = DatFiles.begin(); it != DatFiles.end(); ++it, ++i)
	{
		std::wcout << DatFileNames[i] << '\n';

		for (int i = 0; i < it->NumEntries(); i++)
		{
			std::wcout << L"-- " << (*it)[i]->Name << '\n';
		}
	}
}
