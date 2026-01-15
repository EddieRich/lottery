#ifndef __LOTTERY_DEFINED__
#define __LOTTERY_DEFINED__

#include <raylib.h>
#include <raymath.h>
#include <time.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#pragma pack(1)
typedef struct gamedata_s
{
	time_t first_draw;
	int cost;
	bool has_multiplier;
	int nBalls;
	int nDraw;
	int nBonus;

} GameData_t;
#pragma pack()

#ifndef __LOTTERY_IMPLEMENTATION__
#define EXTERN extern
#else
#define EXTERN

#endif
EXTERN Vector2 render_size;

void set_render_size(void);
void init_graphics(void);
void free_graphics(void);
void render(void);

#endif // __LOTTERY_DEFINED__
