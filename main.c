#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <termios.h>
#include <curl/curl.h>

#include "lottery.h"
#include "util.h"

extern GameData_t gamedata;
bool update = false;
int backtest_count = 0;
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

// sorted holds the "most probable" sorted by score
int sorted[25];
int play_balls[5][5];
void set_play_balls()
{
	// if a future game has 'nDraw' > 5, like KENO, make this bigger
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
			play_balls[i][j] = sorted[i + j * 5];

		sort(play_balls[i], 5);
	}
}

void show_play_balls()
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			printf("%3d", play_balls[i][j]);
		}

		printf("\n");
	}
}

void backtest()
{
	int drawnum = gamedata.last_drawing_number - backtest_count - 1;
	while (drawnum < gamedata.last_drawing_number - 1)
	{
		process_gamedata(drawnum);

		// KENO: will be 20
		int winners[5];
		if (get_winners_for_draw(drawnum + 1, winners) != 0)
			break;

		get_sorted_top_score_balls(sorted);
		set_play_balls(sorted);

		for (int p = 0; p < 5; p++)
		{
			int match = 0;
			for (int b = 0; b < 5; b++)
			{
				if (play_balls[p][b] == winners[b])
					match++;

				printf("%3d", play_balls[p][b]);
			}

			printf(" $1");
			if (match == 3)
			{
				printf(" -> $10");
			}
			if (match == 4)
			{
				printf(" -> $250");
			}
			if (match == 3)
			{
				printf(" -> $100000");
			}

			printf("\n");
		}

		drawnum++;
	}
}

void show_data()
{
	process_gamedata(gamedata.last_drawing_number);

	printf("%s last draw was %s (%d)\n",
		 gamedata.display_name, gamedata.last_drawing_date, gamedata.last_drawing_number);

	int stride = columns / 10;
	if (stride > 7)
		stride = 7;

	int rows = gamedata.nBalls / stride;
	if (rows * stride < gamedata.nBalls)
		rows++;

	// show each balls score
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < gamedata.nBalls; c += rows)
		{
			int i = r + c + 1;
			printf("%2d:%1.3lf  ", i, gamedata.ball_score[i]);
		}

		printf("\n");
	}

	get_sorted_top_score_balls(sorted);
	printf("\n");
	for (int i = 0; i < 25; i++)
		printf("%3d", sorted[i]);

	printf("\n");
	set_play_balls(sorted);
	show_play_balls();
}

int main(int argc, char* argv[])
{
	for (int arg = 1; arg < argc; arg++)
	{
		if (strcmp(argv[arg], "masscash") == 0 || strcmp(argv[arg], "powerball") == 0)
			choose_game(argv[arg]);
		else if (strcmp(argv[arg], "update") == 0)
			update = true;
		else if (strcmp(argv[arg], "backtest") == 0)
			backtest_count = atoi(argv[++arg]);
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

	if (load_lottery() < 0)
		return 3;

	get_terminal_info();
	curl_global_init(CURL_GLOBAL_ALL);

	printf("\x1b[2J\x1b[H");		// clearscreen

	if (update)
	{
		int updates = update_game();
		if (updates < 0)
			return 4;

		printf("%s update added %d drawings\n", gamedata.display_name, updates);
	}

	if (backtest_count > 0)
		backtest();
	else
		show_data();

	if (gamedata.drawings.ptr != NULL)
		free(gamedata.drawings.ptr);

	curl_global_cleanup();

	printf("\n\nGood Luck !!!\n\n");
	return 0;
}
