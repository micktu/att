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

	for (GameFile& gf : gd)
	{
		wc << gf.Path << gf.Filename << std::endl;
	}
}

void DoExtract(int &argc, wchar_t ** &argv)
{
	wstr_t path = add_slash(path_normalize(argv[0]));
	wstr_t outPath = add_slash(path_normalize(argv[1]));

	GameData gd(path);
	ReadGameData(gd, argc > 2 ? argv[2] : L"", L"Extracting");

	std::vector<DatFile>& datFiles = gd.GetDatFiles();

	for (GameFile& gf : gd)
	{
		if (gf.DatIndex != ~0)
		{
			wstr_t outDir = outPath + add_slash(path_strip_filename(strip_slash(gf.Path)));
			create_dir_recursive(outDir);
			DatFile& dat = datFiles[gf.DatIndex];
			dat.ExtractFile(&dat[gf.IndexInDat], outDir);
		}
	}
}

static const std::wregex SUB_REGEX(L"\\\\([a-zA-Z0-9]+)_?([a-zA-Z]+)?\\.dat$");

void ProcessSubFile(GameFile *gf, std::map<std::wstring, sub_content>& subs)
{
	if (subs.count(gf->Filename) < 1)
	{
		subs.emplace(gf->Filename, sub_content());
	}
	sub_content &sc = subs[gf->Filename];

	DatFileEntry* dat = gf->GetResource();
	wstr_t datPath(dat->Dat->GetPath());


	std::wsmatch match;
	std::regex_search(datPath, match, SUB_REGEX);
	const wstr_t &loc = match[2].str();

	uint32_t numEntries;
	std::ifstream* file = gf->GetResource()->OpenFile();
	file->read((char *)&numEntries, sizeof(uint32_t));

	wstr_t id(0x44, 0);
	wstr_t val(0x400, 0);
	for (uint32_t i = 0; i < numEntries; i++)
	{
		file->read((char*)id.c_str(), 0x44 * sizeof(wchar_t));
		id.resize(wcslen(id.c_str()));

		file->read((char*)val.c_str(), 0x400 * sizeof(wchar_t));
		val.resize(wcslen(val.c_str()));

		if (sc.count(id) < 1)
		{
			sc.emplace(id, LocMessage());
			sc[id].Id = wstr_to_utf8(id);
		}

		LocMessage &line = sc[id];
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
		else
		{
			wc << L"WARNING: unknown suffix " << loc << L" in " << dat->Name;
		}
	}

	delete file;
}

void ExportSubs(std::vector<GameFile*> &files, wstr_t outDir)
{
	std::map<std::wstring, sub_content> subs;

	for (GameFile* file : files)
	{
		ProcessSubFile(file, subs);
	}

	for (auto& pair : subs)
	{

	}

	return;
}

void ExportBinFile(GameFile &gf, wstr_t outPath)
{
	return;

	DatFileEntry* dat = gf.GetResource();
	char *bin = dat->ReadFile();
	ScriptContent* content = script_extract(bin);


	wstr_t outDir = outPath + add_slash(path_strip_filename(strip_slash(gf.Path)));
	create_dir_recursive(outDir);
	outDir += gf.Filename;

	//script_dump_debug(bin, outDir + L".debug.txt", content);

	if (nullptr != content)
	{
		script_export(content, outDir + L".txt");
		wc << L"Done." << std::endl;
	}
	else
	{
		wc << L"Failed to load script." << std::endl;
	}

	delete content;
	delete bin;
}

void DoExport(int &argc, wchar_t ** &argv)
{
	wstr_t path = add_slash(path_normalize(argv[0]));
	const wchar_t* outp = argc > 1 ? argv[1] : L"out";
	wstr_t outPath = add_slash(path_normalize(outp));

	GameData gd(path);
	ReadGameData(gd, L"text", L"Exporting");

	std::vector<GameFile*> subFiles;

	for (GameFile& gf : gd)
	{
		wc << L"Processing " << gf.Filename << L"... ";

		if (ext_equals(gf.Filename, L".bin"))
		{
			ExportBinFile(gf, outPath);
		}
		else if (ext_equals(gf.Filename, L".smd"))
		{
			subFiles.push_back(&gf);
		}
	}

	ExportSubs(subFiles, outPath);

	wc << std::endl << L"Fin";
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