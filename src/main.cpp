#include "main.h"

#include <regex>
#include <conio.h>

#include "GameData.h"
#include "script.h"
#include "utils.h"


int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	if (argc < 2)
	{
		DoHelp();
		return 0;
	}

	auto it = commands.find(argv[1]);
	if (it == commands.end())
	{
		DoHelp();
		return 0;
	}

	int ac = argc - 2;
	wchar_t ** av = argv + 2;
	it->second(ac, av);

	//while (!_kbhit()) {}
	return 0;
}

void DoHelp()
{
	wc << std::endl << L"NieR: Automata Text Tools by @micktu" << std::endl;
	wc << std::endl << L"  Available commands:" << std::endl;
	wc << std::endl << L"  att list <PATH> [FILTER]" << std::endl;
	wc << L"      Lists all files in a directory or .dat container specified by <PATH>." << std::endl;
}

void DoList(int &argc, wchar_t ** &argv)
{
	wstr_t path = add_slash(path_normalize(argv[0]));

	GameData gd(path);
	ReadGameData(gd, argc > 1 ? argv[1] : L"", L"Listing");

	for (auto &pair : gd)
	{
		GameFile &gf = pair.second;
		wc << gf.Path << gf.Filename << std::endl;
	}
}

void DoExtract(int &argc, wchar_t ** &argv)
{
	wstr_t path = add_slash(path_normalize(argv[0]));
	wstr_t outPath = add_slash(path_normalize(argv[1]));

	GameData gd(path);
	ReadGameData(gd, argc > 2 ? argv[2] : L"", L"Extracting");

	std::vector<DatFile> &datFiles = gd.GetDatFiles();

	for (auto &pair : gd)
	{
		GameFile& gf = pair.second;
		if (gf.DatIndex != ~0)
		{
			wstr_t outDir = outPath + add_slash(path_strip_filename(strip_slash(gf.Path)));
			create_dir_recursive(outDir);
			DatFile& dat = datFiles[gf.DatIndex];
			dat.ExtractFile(dat[gf.IndexInDat], outDir);
		}
	}
}

static const std::wregex STR_FILE_REGEX(L"\\\\\\w+?(_([a-zA-Z]{2}))?\\.dat");
static wchar_t WCS_BUFFER[0x2B2B];

void ProcessStringFile(GameFile *gf, std::map<std::wstring, mess_map> *strMap)
{
	DatFileEntry* dat = gf->GetDatEntry();
	wstr_t datPath(dat->Dat->GetPath());

	std::wsmatch match;
	std::regex_search(datPath, match, STR_FILE_REGEX);
	const wstr_t &loc = match[2].str();

	if (strMap->count(gf->Filename) < 1)
	{
		strMap->emplace(gf->Filename, mess_map());
	}
	mess_map &mesMap = strMap->at(gf->Filename);

	uint32_t numEntries;
	std::ifstream file = gf->GetDatEntry()->OpenFile();
	file.read((char *)&numEntries, sizeof(uint32_t));

	bool bIsVarLength = ext_equals(gf->Filename, L".tmd");
	uint32_t idLen = 0x44, valLen = 0x400;
	wstr_t id, val;
	std::map<wstr_t, wstr_t> lines;

	for (uint32_t i = 0; i < numEntries; i++)
	{
		if (bIsVarLength) file.read((char *)&idLen, sizeof(uint32_t));
		file.read((char*)WCS_BUFFER, idLen * sizeof(wchar_t));
		id.assign(WCS_BUFFER);

		if (bIsVarLength) file.read((char *)&valLen, sizeof(uint32_t));
		file.read((char*)WCS_BUFFER, valLen * sizeof(wchar_t));
		val.assign(WCS_BUFFER);

		if (mesMap.count(id) < 1)
		{
			mesMap.emplace(id, LocMessage());
			mesMap[id].Id = wstr_to_utf8(id);
		}
		LocMessage &line = mesMap[id];

		str_t text = wstr_to_utf8(val);

		if (loc.empty())
		{
			line.Jp = text;
		}
		else if (loc.compare(L"us") == 0)
		{
			line.En = text;
		}
		else if (loc.compare(L"fr") == 0)
		{
			line.Fr = text;
		}
		else if (loc.compare(L"it") == 0)
		{
			line.It = text;
		}
		else if (loc.compare(L"de") == 0)
		{
			line.De = text;
		}
		else if (loc.compare(L"es") == 0)
		{
			line.Sp = text;
		}
		else if (loc.compare(L"kr") == 0)
		{
			line.Kr = text;
		}
		else if (loc.compare(L"cn") == 0)
		{
			line.Cn = text;
		}
		else
		{
			wc << L"WARNING: unknown suffix " << loc << L" in " << dat->Name << std::endl;
		}
	}
}

void ExportStrings(std::vector<GameFile*> &files, wstr_t outPath)
{
	auto *strMap = new std::map<std::wstring, mess_map>();

	for (GameFile* file : files)
	{
		ProcessStringFile(file, strMap);
	}
	
	wstr_t strDir = outPath + L"txtmess\\";
	create_dir_recursive(strDir);
	
	wstr_t subDir = outPath + L"subtitle\\";
	create_dir_recursive(subDir);
	
	std::ofstream file;

	size_t strCount = 0, subCount = 0;

	for (auto& pair : *strMap)
	{
		wstr_t filename;
		if (ext_equals(pair.first, L".tmd"))
		{
			strCount += pair.second.size();
			filename.assign(strDir);
		}
		else
		{
			subCount += pair.second.size();
			filename.assign(subDir);
		}

		filename += pair.first;
		filename += L".txt";

		file.open(filename);
		for (auto& mesPair : pair.second)
		{
			file << format_loc_message(mesPair.second) << std::endl;
		}

		file.close();
	}

	wc << "String lines: " << strCount << std::endl;
	wc << "Subtitle lines: " << subCount << std::endl;

	delete strMap;
}

size_t ExportBinFile(GameFile *gf, wstr_t outPath)
{
	DatFileEntry* dat = gf->GetDatEntry();
	char_vector_t bin = dat->ReadFile();
	ScriptContent* content = script_extract(bin.data(), gf->Filename);

	//wstr_t outDir = outPath + add_slash(path_strip_filename(strip_slash(gf->Path)));
	wstr_t outDir = outPath + gf->Path;
	wstr_t outFilename = outDir + gf->Filename;

	//script_dump_debug(bin, outDir + L".debug.txt", content);

	size_t count = 0;

	if (nullptr != content)
	{
		count = content->Messages.size();
		
		if (count > 0)
		{
			create_dir_recursive(outDir);
			script_export(content, outFilename + L".txt");
		}
	}
	else
	{
		wc << outFilename << L" failed to load." << std::endl;
	}

	delete content;

	return count;
}

void ExportScripts(std::vector<GameFile*> &files, wstr_t outPath)
{
	size_t count = 0;

	for (GameFile* gf : files)
	{
		count += ExportBinFile(gf, outPath);
	}

	wc << L"Script lines: " << count << std::endl;
}

void DoExport(int &argc, wchar_t ** &argv)
{
	wstr_t path = add_slash(path_normalize(argv[0]));
	const wchar_t* outp = argc > 1 ? argv[1] : L"out";
	wstr_t outPath = add_slash(path_normalize(outp));

	GameData gd(path);
	ReadGameData(gd, L"text", L"Exporting");

	//wc << L"Reading files..." << std::endl;

	std::vector<GameFile*> scriptFiles;
	std::vector<GameFile*> stringFiles;

	for (auto &pair : gd)
	{
		GameFile& gf = pair.second;

		if (ext_equals(gf.Filename, L".bin"))
		{
			scriptFiles.push_back(&gf);
		}
		else if (ext_equals(gf.Filename, L".smd") || ext_equals(gf.Filename, L".tmd"))
		{
			stringFiles.push_back(&gf);
		}
	}

	wc << L"Saving scripts..." << std::endl;
	ExportScripts(scriptFiles, outPath);
	wc << std::endl;

	wc << L"Saving strings..." << std::endl;
	ExportStrings(stringFiles, outPath);
	wc << std::endl;

	wc << L"All done.";
}

str_map_t LoadText(wstr_t &path)
{
	std::ifstream file(path);

	str_map_t messages;
	str_t id;
	for (str_t line; getline(file, line);)
	{
		if (line.length() < 6) continue;

		str_t prefix = line.substr(0, 4);
		str_t payload = line.substr(4, line.length() - 2);

		if (prefix.compare("ID: ") == 0)
		{
			id = payload.c_str();
		}
		else if (prefix.compare("RU: ") == 0)
		{
			if (messages.count(id) > 0)
			{
				std::cout << id << " translation already exists.";
			}

			messages.emplace(id, payload);
		}
	}

	return messages;
}

void DoImport(int &argc, wchar_t **&argv)
{
	wstr_t dataPath = add_slash(path_normalize(argv[0]));
	wstr_t inPath = add_slash(path_normalize(argv[1]));
	const wchar_t* outp = argc > 2 ? argv[2] : L"patch";
	wstr_t outPath = add_slash(path_normalize(outp));

	GameData gd(dataPath);
	ReadGameData(gd, L"text", L"Patching");

	wstr_vec_t files = find_files(inPath);
	
	for (wstr_t &fullName : files)
	{
		str_map_t messages = LoadText(inPath + fullName);
		if (messages.size() < 1) continue;

		wstr_t realName = fullName.substr(0, fullName.length() - 4);
		wstr_t path, filename, ext;
		split_path(realName, path, filename, ext);

		if (ext_equals(ext, L".bin"))
		{
			GameFile &gf = gd[realName];

			char_vector_t bin = gf.GetDatEntry()->ReadFile();
			char_vector_t patchedBin = script_import(messages, bin.data(), filename);

			// TODO Create dir; Copy dat; cache dat; write patch int dat

		}
		else if (ext_equals(ext, L".tmd") || ext_equals(ext, L".smd"))
		{

		}
	}
}

void ReadGameData(GameData& gd, const wstr_t filter, const wstr_t verb)
{
	size_t numFiles = gd.GetFilenames().size();
	wc << L"Checking " << numFiles << L" files in " << gd.GetBasePath() << L"... ";
	gd.Read(filter);

	size_t numGameFiles = gd.GetGameFiles().size();
	size_t numDatFiles = gd.GetDatFiles().size();
	wc << L"Done." << std::endl;
	if (!verb.empty()) wc << verb << L" ";
	wc << numGameFiles << L" files in " << numDatFiles << L" archives." << std::endl;
	wc << std::endl;
}