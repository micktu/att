// att.cpp : Defines the entry point for the console application.
//

#include "stdafx.h";

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
	
	dat_file dat = dat_open(argv[1]);

	if (dat.num_files == 0) return 1;

	printf("DAT file %s OK.\n\n ", argv[1]);

	char dat_path[MAX_PATH], dat_filename[MAX_PATH], out_dir[MAX_PATH] = "out";
	split_path_file(dat_path, dat_filename, argv[1]);
	CreateDirectoryA(out_dir, NULL);

	sprintf_s(out_dir, "%s\\%.*s", out_dir, (int)strlen(dat_filename) - 4, dat_filename);
	CreateDirectoryA(out_dir, NULL);

	sprintf_s(out_dir, "%s\\scripts", out_dir);
	CreateDirectoryA(out_dir, NULL);

	for (int i = 0; i < dat.num_files; i++)
	{
		if (dat.extensions[i].num != '\x00nib') continue;

		printf("Processing script %s... ", dat.names[i]);

		dat_seek(dat, i);
		script_content* content = script_extract(dat.file);

		char out_path[MAX_PATH];
		sprintf_s(out_path, "%s\\%.*s.txt", out_dir, (int)strlen(dat.names[i]) - 4, dat.names[i]);

		script_export(content, out_path);

		//strcpy(out_path + strlen(out_path) - 3, "debug.txt");
		//dat_seek(dat, i);
		//script_export_debug(dat.file, out_path);

		printf("Done.\n");
	}

	dat_close(dat);

	return 0;
}

