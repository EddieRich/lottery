#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <termios.h>
#include <curl/curl.h>

#include "lottery.h"
#include "parser.h"
#include "util.h"

GameData_t gamedata = { 0 };
bool update = false;
int lines, columns;

void choose_game(char* gamename)
{
	if (strcmp(gamename, "masscash") == 0)
	{
		sprintf(gamedata.display_name, "Mass Cash");
		sprintf(gamedata.identifier, "mass_cash");
		sprintf(gamedata.name, "masscash");
		sprintf(gamedata.first_draw, "1991-03-28");
		gamedata.cost = 1;
		gamedata.multiplier_cost = 0;
		gamedata.nBalls = 35;
		gamedata.nDraw = 5;
		gamedata.nBonus = 0;
	}
	else
		memset(&gamedata, 0, sizeof(GameData_t));
}

int get_top_score_index(double score[70])
{
	double max = -1.0;
	int index = -1;
	for (int i = 0; i < 70; i++)
	{
		if (score[i] > max)
		{
			max = score[i];
			index = i;
		}
	}

	return index;
}

int* get_sorted_numbers(int size)
{
	// make a copy of the scores
	double score[70];
	for (int i = 0; i < 70; i++)
		score[i] = gamedata.ball_score[i];

	// recursively get the top score index 
	int* p = calloc(size, sizeof(int));
	for (int i = 0; i < size; i++)
	{
		int s = get_top_score_index(score);
		score[s] = 0.0;
		p[i] = s;
	}

	return p;
}

void get_terminal_info()
{
	struct termios term;
	tcgetattr(0, &term);
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSAFLUSH, &term);

	printf("\x1b[999;999H");
	fflush(stdout);
	printf("\x1b[6n");
	fflush(stdout);
	scanf("\x1b[%d;%dR", &lines, &columns);

	term.c_lflag |= (ICANON | ECHO);
	tcsetattr(0, TCSAFLUSH, &term);
}

int main(int argc, char* argv[])
{
	for (int arg = 1; arg < argc; arg++)
	{
		if (strcmp(argv[arg], "masscash") == 0 || strcmp(argv[arg], "powerball") == 0)
			choose_game(argv[arg]);
		else if (strcmp(argv[arg], "update") == 0)
			update = true;
		else
		{
			printf("I don't understand : %s\n", argv[arg]);
			return 1;
		}
	}

	if (gamedata.display_name[0] == 0)
	{
		printf("No game specified\n");
		return 2;
	}

	get_terminal_info();

	curl_global_init(CURL_GLOBAL_ALL);

	printf("\x1b[2J\x1b[H");

	if (update)
		printf("%s update added %d drawings\n", gamedata.display_name, update_game());

	parse_game();

	printf("%s %d drawings, last draw was %s (%d)\n",
		 gamedata.display_name, gamedata.num_drawings, gamedata.last_drawing_date, gamedata.last_drawing_number);

	int stride = columns / 10;
	if (stride > 7)
		stride = 7;

	int rows = gamedata.nBalls / stride;
	if (rows * stride < gamedata.nBalls)
		rows++;

	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < gamedata.nBalls; c += rows)
		{
			int i = r + c + 1;
			printf("%2d:%1.3lf  ", i, gamedata.ball_score[i]);
		}

		printf("\n");
	}

	int sz = gamedata.nDraw * gamedata.nDraw;
	int* sorted = get_sorted_numbers(sz);

	printf("\n");
	for (int i = 0; i < sz; i++)
		printf("%3d", sorted[i]);

	printf("\n");

	// if a future game has more 'nDraw', like keno, make this bigger
	int winner[5];
	for (int i = 0; i < gamedata.nDraw; i++)
	{
		printf("\n");
		for (int j = 0; j < gamedata.nDraw; j++)
			winner[j] = sorted[i + j * gamedata.nDraw];

		sort(winner, 5);
		for (int j = 0; j < 5; j++)
			printf("%3d", winner[j]);
	}

	free(sorted);

	printf("\n\nGood Luck !!!\n\n");

	curl_global_cleanup();
	return 0;
}
