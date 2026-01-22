#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "parser.h"
#include "util.h"

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
		char* pNum = strAfter(line, "drawNumber\":");
		char* pWin = strAfter(line, "winningNumbers\":[");
		pDate[indexOf(pDate, '"')] = 0;
		pNum[indexOf(pNum, ',')] = 0;
		pWin[indexOf(pWin, ']')] = 0;

		fprintf(fp, "%s,%s,%s\n", pDate, pNum, pWin);
	}

	fclose(fp);
	fclose(fpjson);
}
