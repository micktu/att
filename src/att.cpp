// att.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "utils.h"
#include "DatFile.h"
#include "script.h"

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

	delete buffer;
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
	/*
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
	*/
}

int main(int argc, char* argv[])
{
	path_vector_t files = find_files(L"..\\data\\");

	for (std::wstring s : files)
	{
		std::wcout << s << '\n';
	}

	while (!_kbhit()) {}
	return 0;

	if (argc < 2) return 0;

	char* command = argv[1];

	if (strcmp(command, "export") == 0)
	{
		char* datFullPath = argv[2];
		char* outDir = argv[3];

		//path_vector files = find_files_recursive();

		bool bDebugEnabled = false;
		if (argc > 4)
		{
			bDebugEnabled = strcmp(argv[4], "debug") == 0;
		}

		//do_export(datFullPath, outDir, bDebugEnabled);
		return 0;
	}

	if (strcmp(command, "extract") == 0)
	{
		char* filename = argv[2];
		char* outPath = argv[3];

		//do_extract(filename, outPath);
		return 0;
	}

	if (strcmp(command, "list") == 0)
	{
		char* filename = argv[2];

		//do_list(filename);
		return 0;
	}

	return 0;
}

