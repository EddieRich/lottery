#ifndef __LOTTERY_DEFINED__
#define __LOTTERY_DEFINED__

#pragma pack(1)

typedef struct gamedata_s
{

	char display_name[20];
	char identifier[20];
	char first_draw[20];
	char bonus_ball_name[20];
	char miltiplier_name[20];
	int cost;
	int multiplier_cost;
	int nBalls;
	int nDraw;
	int nBonus;

	int num_drawings;
	char last_entry_date[11];
	int ball_times_drawn[70];
	int ball_last_drawn[70];
	double ball_score[70];
	int bonus_times_drawn[27];
	int bonus_last_drawn[27];
	double bonus_score[70];

} GameData_t;
#pragma pack()

#endif // __LOTTERY_DEFINED__
