#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lottery.h"
#include "parser.h"

char* strAfter(char* haystack, char* needle)
{
	char* found = strstr(haystack, needle);
	if (found != NULL)
		found += strlen(needle);

	return found;
}

int indexOf(char* haystack, char needle)
{
	int index = -1;
	char* found = strchr(haystack, needle);
	if (found != NULL)
		index = found - haystack;

	return index;
}

void parse_json(char* name)
{
	char filepath[FILENAME_MAX];
	char line[500];

	sprintf(filepath, "/home/ed/lottery/%s.json", name);
	FILE* fpjson = fopen(filepath, "rb");
	sprintf(filepath, "/home/ed/lottery/%s.lottery", name);
	FILE* fp = fopen(filepath, "wb");

	while (!feof(fpjson))
	{
		memset(line, 0, 500);
		fgets(line, 500, fpjson);

		if (strlen(line) < 100)
			break;

		char* pDate = strAfter(line, "drawDate\":\"");
		char* pWin = strAfter(line, "winningNumbers\":[");
		pDate[indexOf(pDate, '"')] = 0;
		pWin[indexOf(pWin, ']')] = 0;

		fprintf(fp, "%s:%s\n", pDate, pWin);
	}

	fclose(fp);
	fclose(fpjson);
}

void parse(char* name, GameMatrix_t* pgm)
{
	// parse_json(name);

	char filepath[FILENAME_MAX];

	sprintf(filepath, "/home/ed/lottery/%s.lottery", name);
	FILE* fp = fopen(filepath, "rb+");
	if (fp == NULL)
		return;

	fclose(fp);
}
