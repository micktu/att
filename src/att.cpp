// att.cpp : Defines the entry point for the console application.
//

#include "stdafx.h";

#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <limits.h>

#include "dat.h"
#include "script.h"


void split_path_file(char* p, char* f, char *pf) {
	char *slash = pf, *next;
	while ((next = strpbrk(slash + 1, "\\/"))) slash = next;
	if (pf != slash) slash++;

	strncpy_s(p, slash - pf + 1, pf, slash - pf);
	strcpy_s(f, strlen(slash) + 1, slash);
}

int main(int argc, char* argv[])
{
	if (argc != 2) return 1;
	
	char* datFullPath = argv[1];

	DatFile dat(datFullPath);

	if (dat.NumEntries() == 0) return 1;

	char dat_path[MAX_PATH], dat_filename[MAX_PATH];
	split_path_file(dat_path, dat_filename, datFullPath);
	printf("DAT file %s is OK.\n\n ", dat_filename);

	char out_dir[MAX_PATH] = "out";
	CreateDirectoryA(out_dir, NULL);

	sprintf_s(out_dir, "%s\\%.*s", out_dir, (int)strlen(dat_filename) - 4, dat_filename);
	CreateDirectoryA(out_dir, NULL);

	sprintf_s(out_dir, "%s\\scripts", out_dir);
	CreateDirectoryA(out_dir, NULL);

	DatFileEntry* entry;

	for (int i = 0; i < dat.NumEntries(); i++)
	{
		entry = dat[i];

		if (entry->Extension.num != '\x00nib') continue;

		printf("Processing script %s... ", entry->Name);

		char* buffer = new char[entry->Size];
		dat.ReadFile(i, buffer);
	
		FILE* tempFile = tmpfile();
		fwrite(buffer, entry->Size, 1, tempFile);
		rewind(tempFile);
		script_content* content = script_extract(tempFile);
		fclose(tempFile);
		delete buffer;

		char out_path[MAX_PATH];
		sprintf_s(out_path, "%s\\%.*s.txt", out_dir, (int)strlen(entry->Name) - 4, entry->Name);

		script_export(content, out_path);

		//strcpy(out_path + strlen(out_path) - 3, "debug.txt");
		//dat_seek(dat, i);
		//script_export_debug(dat.file, out_path);

		printf("Done.\n");
	}

	return 0;
}

