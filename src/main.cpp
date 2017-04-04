#include "main.h"

#include <conio.h>

#include "GameData.h"
#include "script.h"
#include "utils.h"


int wmain(int argc, wchar_t *argv[])
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

static wchar_t WCS_BUFFER[0x2B2B];

std::map<std::wstring, mess_map> LoadStrings(std::vector<GameFile*> files)
{
	std::map<std::wstring, mess_map> strMap;
	
	for (GameFile* gf : files)
	{

		DatFileEntry* dat = gf->GetDatEntry();
		wstr_t datPath(dat->Dat->GetPath());

		wstr_t loc = find_filename_suffix(datPath);

		if (strMap.count(gf->Filename) < 1)
		{
			strMap.emplace(gf->Filename, mess_map());
		}
		mess_map &mesMap = strMap.at(gf->Filename);

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
	
	return strMap;
}

void ExportStrings(std::vector<GameFile*> &files, wstr_t outPath)
{
	std::map<std::wstring, mess_map> strMap = LoadStrings(files);
	
	wstr_t strDir = outPath + L"txtmess\\";
	create_dir_recursive(strDir);
	
	wstr_t subDir = outPath + L"subtitle\\";
	create_dir_recursive(subDir);
	
	std::ofstream file;

	size_t strCount = 0, subCount = 0;

	for (auto& pair : strMap)
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
	const wchar_t* datap = argc > 0 ? argv[0] : L"data";
	wstr_t dataPath = add_slash(path_normalize(datap));
	const wchar_t* outp = argc > 1 ? argv[1] : L"text";
	wstr_t outPath = add_slash(path_normalize(outp));

	GameData gd(dataPath);
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

str_map_t LoadText(wstr_t path)
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

			messages.emplace(id, lit_to_lb(payload));
		}
	}

	return messages;
}

void DoImport(int &argc, wchar_t **&argv)
{
	const wchar_t* datap = argc > 0 ? argv[0] : L"data";
	wstr_t dataPath = add_slash(path_normalize(datap));
	const wchar_t* inp = argc > 1 ? argv[1] : L"text";
	wstr_t inPath = add_slash(path_normalize(inp));
	const wchar_t* outp = argc > 2 ? argv[2] : L"patch";
	wstr_t outPath = add_slash(path_normalize(outp));

	GameData gd(dataPath);
	ReadGameData(gd, L"text", L"Patching");

	std::map<DatFile*, std::map<uint32_t, char_vector_t>> patches;
	wstr_vec_t files = find_files(inPath);

	std::vector<GameFile*> strFiles;
	std::map<std::wstring, str_map_t> strMessages;

	for (wstr_t &fullName : files)
	{
		wstr_t realName = fullName.substr(0, fullName.length() - 4);
		wstr_t path, filename, ext;
		split_path(realName, path, filename, ext);

		str_map_t messages = LoadText(inPath + fullName);
		if (messages.size() < 1) continue;

		if (ext_equals(ext, L".bin"))
		{
			GameFile &gf = gd[realName];

			DatFileEntry &entry = *gf.GetDatEntry();
			char_vector_t bin = entry.ReadFile();
			char_vector_t patchedBin = script_import(messages, bin.data(), filename);

			DatFile *dat = entry.Dat;
			patches.try_emplace(dat);
			patches[dat].emplace(entry.Index, patchedBin);
		}
		else if (ext_equals(ext, L".tmd") || ext_equals(ext, L".smd"))
		{
			wstr_t name = filename.substr(0, filename.find('.'));

			wstr_t strPath = path + name;
			strPath += L"_us.dat\\";
			strPath += filename;

			if (gd.GetGameFiles().count(strPath) > 0)
			{
				GameFile &gf = gd[strPath];
				strFiles.emplace_back(&gf);
				strMessages.emplace(filename, messages);
			}
			else
			{
				wc << strPath << L" does not exist.";
			}
		}
	}

	std::map<std::wstring, mess_map> strings = LoadStrings(strFiles);
	str_map_t outStrings;

	for (auto &fileMapPair : strings)
	{
		str_map_t &ruMessages = strMessages[fileMapPair.first];
		for (auto &idMesPair : fileMapPair.second)
		{
			str_t id = wstr_to_utf8(idMesPair.first);
			if (ruMessages.count(id) < 1)
			{
				ruMessages.emplace(id, idMesPair.second.En);
			}
		}
	}

	for (auto &gf : strFiles)
	{
		auto &messages = strMessages[gf->Filename];
		auto entry = gf->GetDatEntry();
		auto &dat = entry->Dat;

		bool bVarLength = ext_equals(gf->Extension, L".tmd");
		
		std::ostringstream stream(std::ios::binary);
		
		uint32_t numEntries = (uint32_t)messages.size();
		stream.write((char*)&numEntries, sizeof(uint32_t));

		uint32_t idLen = 0x44;
		uint32_t valLen = 0x400;
		for (auto &pair : messages)
		{
			wstr_t id = utf8_to_wstr(pair.first.data());
			wstr_t val = utf8_to_wstr(pair.second.data());

			if (bVarLength)
			{
				idLen = (uint32_t)id.length() + 1;
				stream.write((char*)&idLen, sizeof(uint32_t));
			}
			stream.write((char*)id.data(), idLen * sizeof(wchar_t));

			if (bVarLength)
			{
				valLen = (uint32_t)val.length() + 1;
				stream.write((char*)&valLen, sizeof(uint32_t));
			}
			stream.write((char*)val.data(), valLen * sizeof(wchar_t));
		}

		patches.try_emplace(dat);
		auto str = stream.str();
		patches[dat].emplace(entry->Index, char_vector_t(str.begin(), str.end()));
	}

	for (auto &fileMapPair : patches)
	{
		DatFile *dat = fileMapPair.first;

		create_dir_recursive(outPath + path_strip_filename(dat->GetFilename()));
		wstr_t datPath = outPath + dat->GetFilename();
		CopyFile(dat->GetPath().data(), datPath.data(), false);

		DatFile newDat(outPath, dat->GetFilename());
		for (auto &gpair : fileMapPair.second)
		{
			auto &str = gpair.second;
			newDat.InjectFile(gpair.first, char_vector_t(str.begin(), str.end()));
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