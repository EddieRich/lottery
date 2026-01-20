#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lottery.h"
#include "parser.h"
#include "util.h"

extern GameData_t gamedata;

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

		fprintf(fp, "%s,%s\n", pDate, pWin);
	}

	fclose(fp);
	fclose(fpjson);
}

void parse_game(char* name)
{
	//parse_json(name);
	//return;

	int max_ball_times = 0;
	int min_ball_times = 0x7fffffff;
	int max_ball_last = 0;

	char filepath[FILENAME_MAX];
	sprintf(filepath, "/home/ed/lottery/%s.lottery", name);

	FILE* fp = fopen(filepath, "rb");
	if (fp == NULL)
		return;

	gamedata.num_drawings = 0;
	for (int i = 0; i < 70; i++)
	{
		gamedata.ball_times_drawn[i] = 0;
		gamedata.ball_last_drawn[i] = 0;
		gamedata.ball_score[i] = 0.0;
	}

	for (int i = 0; i < 27; i++)
	{
		gamedata.bonus_times_drawn[i] = 0;
		gamedata.bonus_last_drawn[i] = 0;
		gamedata.bonus_score[i] = 0.0;
	}

	char line[40];
	while (fgets(line, 40, fp) != NULL)
	{
		// incement all last_drawn, 1 based index
		for (int i = 1; i <= gamedata.nBalls; i++)
			gamedata.ball_last_drawn[i]++;

		if (gamedata.nBonus)
		{
			for (int i = 1; i <= gamedata.nBonus; i++)
				gamedata.bonus_last_drawn[i]++;
		}

		char* p = &line[11];
		for (int b = 0; b < gamedata.nDraw; b++)
		{
			int i = indexOf(p, ',');
			if (i < 0)
				i = indexOf(p, '\n');

			p[i] = 0;
			int ball = atoi(p);
			p += i + 1;

			// 1 based index
			gamedata.ball_last_drawn[ball] = 0;
			gamedata.ball_times_drawn[ball]++;
		}

		if (gamedata.nBonus)
		{
			int i = indexOf(p, ',');
			if (i < 0)
				i = indexOf(p, '\n');

			p[i] = 0;
			int ball = atoi(p);

			// 1 based index
			gamedata.bonus_last_drawn[ball] = 0;
			gamedata.bonus_times_drawn[ball]++;
		}

		strncpy(gamedata.last_entry_date, line, 10);
		gamedata.num_drawings++;
	}

	fclose(fp);

	// now to the scores, balls first
	// find max and min for remap
	for (int i = 1; i <= gamedata.nBalls; i++)
	{
		if (gamedata.ball_times_drawn[i] > max_ball_times)
			max_ball_times = gamedata.ball_times_drawn[i];
		if (gamedata.ball_times_drawn[i] < min_ball_times)
			min_ball_times = gamedata.ball_times_drawn[i];
		if (gamedata.ball_last_drawn[i] > max_ball_last)
			max_ball_last = gamedata.ball_last_drawn[i];
	}

	double min_score = 2.0;
	double max_score = 0.0;

	for (int i = 1; i <= gamedata.nBalls; i++)
	{
		gamedata.ball_score[i] = remap(min_ball_times, max_ball_times, 1.0, 0.0, gamedata.ball_times_drawn[i]);
		gamedata.ball_score[i] += remap(0, max_ball_last, 0.0, 1.0, gamedata.ball_last_drawn[i]);

		if (gamedata.ball_score[i] < min_score)
			min_score = gamedata.ball_score[i];

		if (gamedata.ball_score[i] > max_score)
			max_score = gamedata.ball_score[i];
	}

	for (int i = 1; i <= gamedata.nBalls; i++)
		gamedata.ball_score[i] = remap(min_score, max_score, 0.0, 1.0, gamedata.ball_score[i]);
}
