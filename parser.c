#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "lottery.h"
#include "parser.h"
#include "util.h"

extern GameData_t gamedata;

#pragma pack(1)
struct dynptr_s
{
	char* ptr;
	size_t size;
};
#pragma pack()

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	struct dynptr_s* dp = (struct dynptr_s*)userp;

	dp->ptr = realloc(dp->ptr, dp->size + realsize + 1L);

	memcpy(&dp->ptr[dp->size], contents, realsize);
	dp->size += realsize;
	dp->ptr[dp->size] = 0;

	return realsize;
}

int update_game()
{
	// first, get the last drawing from the database
	char filepath[200];
	sprintf(filepath, "/home/ed/lottery/%s.lottery", gamedata.name);
	FILE* fp = fopen(filepath, "rb");
	fseek(fp, -40L, SEEK_END);
	char line[50];
	memset(line, 0, 50);
	fgets(line, 50, fp);
	fgets(line, 50, fp);
	fclose(fp);

	char last_date[11];
	memset(last_date, 0, 11);
	strncpy(last_date, line, 10);
	line[indexOf(&line[11], ',')] = 0;
	int last_draw_number = atoi(&line[11]);

	// now get todays date
	time_t t = time(NULL);
	struct tm* tm_info = localtime(&t);
	char today[11];           // Buffer for "YYYY-MM-DD\0"
	strftime(today, sizeof(today), "%Y-%m-%d", tm_info); // Format the time

	CURL* handle;
	CURLcode res;
	struct dynptr_s data;

	memset(&data, 0, sizeof(struct dynptr_s));
	handle = curl_easy_init();
	if (handle)
	{
		char url[200];
		sprintf(url,
			"https://masslottery.com/api/v1/draw-results/%s?draw_date_min=%s&draw_date_max%s", gamedata.identifier, last_date, today);
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&data);
		curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
		// some servers don't like requests that are not from a known browser
		curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(handle);
		if (res != CURLE_OK)
		{
			printf("\nURL Failed: %s\n", curl_easy_strerror(res));
			fflush(stdout);
			sprintf(url, "file:///home/ed/lottery/%s_raw.json", gamedata.name);
			curl_easy_setopt(handle, CURLOPT_URL, url);
			res = curl_easy_perform(handle);
		}

		if (res != CURLE_OK)
		{
			printf("File access failed for %s : %s\n\n", url, curl_easy_strerror(res));
			curl_easy_cleanup(handle);
			return 0;
		}

		fp = fopen(filepath, "ab");
		char* p = &data.ptr[21];
		int drawings = 0;
		while (p != NULL)
		{
			// find the end of this drawing data
			char* pend = strstr(p, "},{");
			if (pend != NULL)
			{
				pend[1] = 0;
				pend += 2;
			}

			char* pDate = strAfter(p, "drawDate\":\"");
			char* pNum = strAfter(p, "drawNumber\":");
			char* pWin = strAfter(p, "winningNumbers\":[");
			pDate[indexOf(pDate, '"')] = 0;
			pNum[indexOf(pNum, ',')] = 0;
			pWin[indexOf(pWin, ']')] = 0;

			if (atoi(pNum) > last_draw_number)
				fprintf(fp, "%s,%s,%s\n", pDate, pNum, pWin);

			p = pend;
			drawings++;
		}

		fflush(fp);
		fclose(fp);
		free(data.ptr);
		curl_easy_cleanup(handle);
		return drawings;
	}

	return 0;
}

void parse_json()
{
	char filepath[FILENAME_MAX];
	char line[500];

	sprintf(filepath, "/home/ed/lottery/%s.json", gamedata.name);
	FILE* fpjson = fopen(filepath, "rb");
	sprintf(filepath, "/home/ed/lottery/%s.lottery", gamedata.name);
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

void parse_game()
{
	//parse_json();

	int max_ball_times = 0;
	int min_ball_times = 0x7fffffff;
	int max_ball_last = 0;

	char filepath[FILENAME_MAX];
	sprintf(filepath, "/home/ed/lottery/%s.lottery", gamedata.name);

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
		p[indexOf(p, ',')] = 0;
		gamedata.last_drawing_number = atoi(p);
		p += strlen(p) + 1;
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

		strncpy(gamedata.last_drawing_date, line, 10);
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
