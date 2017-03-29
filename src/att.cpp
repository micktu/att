#include "att.h"

#include <conio.h>

#include "GameData.h"
#include "script.h"


/*
void process_script(DatFileEntry* entry, char* outPath, bool debug)
{
	printf("Processing script %s... ", entry->Name);

	char* buffer = new char[entry->Size];
	entry->Dat->ReadFile(entry->Index, buffer);
	script_content* content = script_extract(buffer);

	char out_path[MAX_PATH];
	sprintf_s(out_path, "%s\\%.*s.txt", outPath, (int)strlen(entry->Name) - 4, entry->Name);

	script_export(content, out_path);

	if (debug)
	{
		sprintf_s(out_path, "%s\\%.*s.debug.txt", outPath, (int)strlen(entry->Name) - 4, entry->Name);
		script_export_debug(buffer, out_path);
	}

	delete[] buffer;
}

void process_subtitle(DatFileEntry* entry, char* outPath)
{
	char* buffer = new char[entry->Size];
	entry->Dat->ReadFile(entry->Index, buffer);

	subtitle_content* subtitle = (subtitle_content*)buffer;

	for (uint32_t i = 0; i < subtitle->numEntries; i++)
	{
		wprintf(L"%s : %s", subtitle->entries[i].id, subtitle->entries[i].text);
	}
}

void do_export_single(wchar_t* datPath, char* outPath, bool debug)
{
	DatFile dat(datPath);

	if (dat.NumEntries() == 0) return;

	char dat_path[MAX_PATH], dat_filename[MAX_PATH];
	split_path_file(dat_path, dat_filename, datPath);
	printf("DAT file %s is OK.\n\n", dat_filename);

	char fullPath[MAX_PATH];
	GetFullPathNameA(outPath, MAX_PATH, fullPath, NULL);
	SHCreateDirectoryExA(NULL, fullPath, NULL);

	char datOutPath[MAX_PATH];
	sprintf_s(datOutPath, "%s\\%.*s", fullPath, (int)strlen(dat_filename) - 4, dat_filename);
	CreateDirectoryA(datOutPath, NULL);

	char scriptOutPath[MAX_PATH];
	sprintf_s(scriptOutPath, "%s\\scripts", datOutPath);
	CreateDirectoryA(scriptOutPath, NULL);

	char subtitleOutPath[MAX_PATH];
	sprintf_s(subtitleOutPath, "%s\\subtitles", datOutPath);
	CreateDirectoryA(subtitleOutPath, NULL);

	DatFileEntry* entry;

	for (int i = 0; i < dat.NumEntries(); i++)
	{
		entry = dat[i];

		if (strcmp(entry->Extension, "bin") == 0)
		{
			process_script(entry, scriptOutPath, debug);
		}
		else if (strcmp(entry->Extension, "smd") == 0)
		{
			process_subtitle(entry, subtitleOutPath);
		}
		else
		{
			continue;
		}

		//if (strcmp(entry->Name, "p100_fa238e7c_scp.bin") == 0)
		//{
		//	char* buffer = new char[entry->Size];
		//	dat.ReadFile(i, buffer);

		//	int binSize;
		//	char* bin = script_import(buffer, "", &binSize);

		//	dat.Replace(i, bin, binSize);
		//	//dat.Save("p100.dat");

		//	delete buffer;
		//}

		printf("Done.\n");
	}

}
*/



int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	if (argc < 2)
	{
		DoHelp();
		return 0;
	}

	str_t verb(argv[1]);

	if (0 == verb.compare(L"list"))
	{
		DoList(argc - 2, argv + 2);
	}
	else
	{
		DoHelp();
	}

	//while (!_kbhit()) {}
	return 0;
}

void DoHelp()
{
	wcout << L"\nNieR: Automata Text Tools by @micktu\n";
	wcout << L"\n  Available commands:\n";
	wcout << L"\n  att list <PATH> [FILTER]\n";
	wcout << L"      Lists all files in a directory or .dat container specified by <PATH>.\n";
}

void DoList(int argc, wchar_t * argv[])
{
	str_t path(argv[0]);
	GameData gd;
	gd.Read(path);

	for (GameFile* gf : gd.GetGameFiles())
	{
		//if (gf->bIsContainer && gf->Files.size() < 1) continue;

		wcout << gf->Filename << L"\n";

		for (GameFile& gfd : gf->Files)
		{
			wcout << L"- " << gfd.Filename << L"\n";
		}

		//if (gf->bIsContainer) wcout << L"\n";
	}
}

