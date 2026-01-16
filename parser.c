#include <curl/curl.h>
#include <stdlib.h>
#include <memory.h>
#include "parser.h"

char* games;
size_t szgames;

void free_games()
{
	if (games != NULL)
		free(games);

	games = NULL;
	szgames = 0UL;
}

void init_parser()
{
	curl_global_init(CURL_GLOBAL_SSL);
	games = NULL;
	szgames = 0UL;
	atexit(free_parser);
}

void free_parser()
{
	curl_global_cleanup();
	free_games();
}

size_t data_received(char* buffer, size_t itemsize, size_t nitems, void* pignore)
{
	size_t bytes = itemsize * nitems;
	games = realloc(games, szgames + bytes);
	memcpy(&games[szgames], buffer, bytes);
	szgames += bytes;

	return bytes;
}

void get_games_meta()
{
	//clear the games buffer first
	free_games();

	CURL* pcurl = curl_easy_init();
	if (pcurl == NULL)
	{
		// TODO: you need error handling
		fprintf(stderr, "curl init failed\n");
		exit(EXIT_FAILURE);
	}

	curl_easy_setopt(pcurl, CURLOPT_URL, "https://masslottery.com/api/v1/games");
	//curl_easy_setopt(pcurl, CURLOPT_URL, "https://dummyjson.com/users");
	curl_easy_setopt(pcurl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
	curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, data_received);

	CURLcode res = curl_easy_perform(pcurl); // Blocking call
	if (res != CURLE_OK)
	{
		FILE* fp = fopen("/home/ed/Projects/bash/lottery/games_raw.json", "rb");
		szgames = 87083UL;
		games = malloc(szgames);
		fread(games, 1, szgames, fp);
		fclose(fp);

		//fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
		//exit(EXIT_FAILURE);
	}
}
