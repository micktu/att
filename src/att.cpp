// att.cpp : Defines the entry point for the console application.
//

#include "stdafx.h";

#include <Shlobj.h>
#include <fcntl.h>
#include <conio.h>

#include "DatFile.h"
#include "script.h"


void split_path_file(char* p, char* f, char *pf) {
	char *slash = pf, *next;
	while ((next = strpbrk(slash + 1, "\\/"))) slash = next;
	if (pf != slash) slash++;

	strncpy_s(p, slash - pf + 1, pf, slash - pf);
	strcpy_s(f, strlen(slash) + 1, slash);
}

void create_dir_recursive(char* path)
{
	char fullPath[MAX_PATH];
	GetFullPathNameA(path, MAX_PATH, fullPath, NULL);
	SHCreateDirectoryExA(NULL, fullPath, NULL);
}

void do_export(char* datPath, char* outPath, bool debug)
{
	DatFile dat(datPath);

	if (dat.NumEntries() == 0) return;

	char dat_path[MAX_PATH], dat_filename[MAX_PATH];
	split_path_file(dat_path, dat_filename, datPath);
	printf("DAT file %s is OK.\n\n", dat_filename);

	char fullPath[MAX_PATH];
	GetFullPathNameA(outPath, MAX_PATH, fullPath, NULL);
	SHCreateDirectoryExA(NULL, fullPath, NULL);

	sprintf_s(fullPath, "%s\\%.*s", fullPath, (int)strlen(dat_filename) - 4, dat_filename);
	CreateDirectoryA(fullPath, NULL);

	sprintf_s(fullPath, "%s\\scripts", fullPath);
	CreateDirectoryA(fullPath, NULL);

	DatFileEntry* entry;

	for (int i = 0; i < dat.NumEntries(); i++)
	{
		entry = dat[i];

		if (entry->Extension.num != '\x00nib') continue;

		printf("Processing script %s... ", entry->Name);

		char* buffer = new char[entry->Size];
		dat.ReadFile(i, buffer);
		script_content* content = script_extract(buffer);

		char out_path[MAX_PATH];
		sprintf_s(out_path, "%s\\%.*s.txt", fullPath, (int)strlen(entry->Name) - 4, entry->Name);

		script_export(content, out_path);

		if (debug)
		{
			sprintf_s(out_path, "%s\\%.*s.debug.txt", fullPath, (int)strlen(entry->Name) - 4, entry->Name);
			script_export_debug(buffer, out_path);
		}

		delete buffer;

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

void do_extract_recursive(char* dirname, char* outPath)
{
	char dirMask[MAX_PATH];
	sprintf_s(dirMask, "%s\\*", dirname);
	
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFileA(dirMask, &ffd);

	if (hFind == INVALID_HANDLE_VALUE) return;
		
	do
	{
		char* name = ffd.cFileName;
		if (name[0] == '.' && (name[1] == '.' || name[1] == 0))
		{
			continue;
		}

		char filename[MAX_PATH];
		sprintf_s(filename, "%s\\%s", dirname, ffd.cFileName);

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			printf("Walking into %s\n", filename);

			char path[MAX_PATH];
			sprintf_s(path, "%s\\%s", outPath, ffd.cFileName);
			CreateDirectoryA(path, NULL);

			do_extract_recursive(filename, path);
			continue;
		}

		if (strcmp(ffd.cFileName + strlen(ffd.cFileName) - 4, ".dat") == 0)
		{
			printf("Extracting %s\n", filename);
			DatFile dat(filename);
			dat.ExtractAll(outPath);
		}
	}
	while (FindNextFileA(hFind, &ffd) != 0);
}

void do_extract(char* filename, char* outPath)
{
	create_dir_recursive(outPath);

	if (strcmp(filename + strlen(filename) - 4, ".dat") == 0)
	{
		DatFile dat(filename);
		dat.ExtractAll(outPath);
	}

	if (filename[strlen(filename) - 1] == '\\')
	{
		filename[strlen(filename) - 1] = 0;
	}

	do_extract_recursive(filename, outPath);
	return;
}

void do_list(char* filename)
{
	DatFile dat(filename);
	dat.PrintFiles();
}

int main(int argc, char* argv[])
{
	if (argc < 2) return 0;

	char* command = argv[1];

	if (strcmp(command, "export") == 0)
	{
		char* datFullPath = argv[2];
		char* outDir = argv[3];

		bool bDebugEnabled = false;
		if (argc > 4)
		{
			bDebugEnabled = strcmp(argv[4], "debug") == 0;
		}

		do_export(datFullPath, outDir, bDebugEnabled);
		return 0;
	}

	if (strcmp(command, "extract") == 0)
	{
		char* filename = argv[2];
		char* outPath = argv[3];

		do_extract(filename, outPath);
		return 0;
	}

	if (strcmp(command, "list") == 0)
	{
		char* filename = argv[2];

		do_list(filename);
		return 0;
	}

	return 0;
}

