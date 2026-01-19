#ifndef __LOTTERY_DEFINED__
#define __LOTTERY_DEFINED__

#include <raylib.h>
#include <raymath.h>
#include <time.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#pragma pack(1)
typedef struct gamematrix_s
{

	char display_name[20];
	char identifier[20];
	char first_draw[20];
	int cost;
	int multiplier_cost;
	int nBalls;
	int nDraw;
	int nBonus;

} GameMatrix_t;
#pragma pack()

#ifndef __LOTTERY_IMPLEMENTATION__
#define EXTERN extern
#else
#define EXTERN

#endif
EXTERN Vector2 render_size;

void set_render_size(void);
void init_lottery(void);
void free_lottery(void);
void render(void);

#endif // __LOTTERY_DEFINED__
