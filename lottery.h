#ifndef __LOTTERY_DEFINED__
#define __LOTTERY_DEFINED__

#pragma pack(1)
struct dynptr_s
{
	char* ptr;
	size_t size;
};

typedef struct gamedata_s
{

	char display_name[20];
	char identifier[20];
	char name[20];
	char first_draw[20];
	char bonus_ball_name[20];		// in extras:{}
	char multiplier_name[20];		// in extras:{}
	int cost;
	int multiplier_cost;
	int nBalls;
	int nDraw;
	int nBonus;

	struct dynptr_s drawings;

	char last_drawing_date[11];
	int last_drawing_number;
	int ball_times_drawn[70];
	int ball_last_drawn[70];
	double ball_score[70];
	int bonus_times_drawn[27];
	int bonus_last_drawn[27];
	double bonus_score[70];

} GameData_t;
#pragma pack()

void process_gamedata(int max_draw_number);
int load_lottery(void);
int update_game(void);
int* get_sorted_top_score_balls(int size);
int get_winners_for_draw(int draw, int* winners);

#endif // __LOTTERY_DEFINED__
