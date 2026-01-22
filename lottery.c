#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "lottery.h"
#include "util.h"

GameData_t gamedata = { 0 };

int load_lottery()
{
	char filepath[FILENAME_MAX];
	sprintf(filepath, "/home/ed/lottery/%s.lottery", gamedata.name);
	FILE* fp = fopen(filepath, "rb");
	if (fp == NULL)
	{
		printf("Error opening %s\n", filepath);
		return -1;
	}

	gamedata.drawings.size = 0L;

	// Seek to the end of the file
	if (fseek(fp, 0L, SEEK_END) == 0)
	{
		// Get the current file position
		long pos = ftell(fp);
		if (pos == -1L)
		{
			printf("Error getting file position : %s\n", filepath);
			fclose(fp);
			return -3;
		}
		gamedata.drawings.size = pos;
	}
	else
	{
		printf("Error seeking to end of file : %s\n", filepath);
		return -2;
	}

	gamedata.drawings.ptr = malloc(gamedata.drawings.size + 1);
	if (gamedata.drawings.ptr == NULL)
	{
		printf("error allocating %zu bytes for %s\n", gamedata.drawings.size, filepath);
		fclose(fp);
		return -4;
	}

	fseek(fp, 0L, SEEK_SET);
	size_t bytes_read = fread(gamedata.drawings.ptr, 1L, gamedata.drawings.size, fp);
	if (bytes_read != gamedata.drawings.size)
	{
		printf("read %zu bytes, expected %zu bytes for %s\n", bytes_read, gamedata.drawings.size, filepath);
		fclose(fp);
		free(gamedata.drawings.ptr);
		gamedata.drawings.ptr = NULL;
		gamedata.drawings.size = -1L;
		return -5;
	}

	gamedata.drawings.ptr[gamedata.drawings.size] = 0;		// null terminate
	fclose(fp);

	// set last draw date and number
	char* pld = strchr(&gamedata.drawings.ptr[gamedata.drawings.size - 45L], '\n') + 1;
	strncpy(gamedata.last_drawing_date, pld, 10);
	pld = strchr(pld, ',') + 1;
	// temporary use of filepath
	memset(filepath, 0, sizeof(filepath));
	strncpy(filepath, pld, indexOf(pld, ','));
	gamedata.last_drawing_number = atoi(filepath);

	return 0;
}

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
	// first, copy the last drawing data from the database
	char last_date[11];
	memset(last_date, 0, 11);
	strncpy(last_date, gamedata.last_drawing_date, 10);
	int last_draw_number = gamedata.last_drawing_number;

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
			"https://masslottery.com/api/v1/draw-results/%s?draw_date_min=%s&draw_date_max=%s", gamedata.identifier, last_date, today);
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&data);
		//curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
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
			return -2;
		}

		char drawing[100];
		char* p = &data.ptr[21];
		int drawings = 0;
		while (p != NULL)
		{
			memset(drawing, 0, 100);
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
			{
				strncpy(gamedata.last_drawing_date, pDate, 10);
				gamedata.last_drawing_number = atoi(pNum);
				sprintf(drawing, "%s,%s,%s\n", pDate, pNum, pWin);
				drawings++;
				size_t draw_size = strlen(drawing);
				gamedata.drawings.ptr = realloc(gamedata.drawings.ptr, gamedata.drawings.size + draw_size + 1L);
				memcpy(&gamedata.drawings.ptr[gamedata.drawings.size], drawing, draw_size);
				gamedata.drawings.size += draw_size;
				gamedata.drawings.ptr[gamedata.drawings.size] = 0;
			}

			p = pend;
		}

		curl_easy_cleanup(handle);
		free(data.ptr);

		if (drawings > 0)
		{
			char filepath[FILENAME_MAX];
			sprintf(filepath, "/home/ed/lottery/%s.lottery", gamedata.name);
			FILE* fp = fopen(filepath, "wb");
			if (fp == NULL)
			{
				printf("Update: Error opening %s for write access\n", filepath);
				return -1;
			}

			fwrite(gamedata.drawings.ptr, 1L, gamedata.drawings.size, fp);
			fclose(fp);
		}

		return drawings;
	}
	else
	{
		printf("curl_easy_init() failed\n");
		return -1;
	}

	return 0;
}

void process_gamedata(int max_draw_number)
{
	// first, clear and reset everything
	int max_ball_times = 0;
	int min_ball_times = 0x7fffffff;
	int max_ball_last = 0;

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

	// now, process line by line
	// TODO: evaluate later for KENO
	char line[100];
	char* p = gamedata.drawings.ptr;
	while (*p != 0)
	{
		// get the next line
		memset(line, 0, sizeof(line));
		int line_length = indexOf(p, '\n');
		strncpy(line, p, line_length);
		p = &p[line_length + 1];

		// incement all last_drawn, 1 based index
		for (int i = 1; i <= gamedata.nBalls; i++)
			gamedata.ball_last_drawn[i]++;

		if (gamedata.nBonus)
		{
			for (int i = 1; i <= gamedata.nBonus; i++)
				gamedata.bonus_last_drawn[i]++;
		}

		char* delim = ",";
		strtok(line, delim);		// skip draw date
		int draw_num = atoi(strtok(NULL, delim));
		for (int b = 0; b < gamedata.nDraw; b++)
		{
			int ball = atoi(strtok(NULL, delim));
			gamedata.ball_last_drawn[ball] = 0;
			gamedata.ball_times_drawn[ball]++;
		}

		if (gamedata.nBonus)
		{
			int ball = atoi(strtok(NULL, delim));
			gamedata.bonus_last_drawn[ball] = 0;
			gamedata.bonus_times_drawn[ball]++;
		}

		if (draw_num >= max_draw_number)
			break;
	}

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

int* get_sorted_top_score_balls(int size)
{
	// make a copy of the scores
	double score[70];
	for (int i = 0; i < 70; i++)
		score[i] = gamedata.ball_score[i];

	// recursively get the top score index 
	int* p = calloc(size, sizeof(int));
	for (int i = 0; i < size; i++)
	{
		double max = -1.0;
		int s = -1;
		for (int j = 0; j < 70; j++)
		{
			if (score[j] > max)
			{
				max = score[j];
				s = i;
			}
		}
		score[s] = 0.0;
		p[i] = s;
	}

	return p;
}

int get_winners_for_draw(int draw, int* winners)
{
	// search backwards for draw number
	char* p = gamedata.drawings.ptr + gamedata.drawings.size - 10L;
	char numbuf[6];
	for (;;)
	{
		while (*p != '\n' && p > gamedata.drawings.ptr)
			p--;

		int i = 12;
		memset(numbuf, 0, sizeof(numbuf));
		for (int j = 0; p[i] != ','; j++, i++)
			numbuf[j] = p[i];

		int num = atoi(numbuf);
		if (num > draw)
			p--;
		else if (num < draw)
		{
			printf("get_winners_for_draw: couldn't find draw number %d\n", draw);
			return -1;
		}
		else
		{
			for (int w = 0; w < 5; w++)
			{
				i++;
				memset(numbuf, 0, sizeof(numbuf));
				for (int j = 0; p[i] != ','; j++, i++)
					numbuf[j] = p[i];

				winners[w] = atoi(numbuf);
			}

			break;
		}
	}

	return 0;
}
