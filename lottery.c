#define __LOTTERY_IMPLEMENTATION__
#include "lottery.h"

const Color bkgnd = { 0, 32, 36, 255 };
const GameData_t* pgd = NULL;

Texture2D texMassCash = { 0 };
Texture2D texPowerball = { 0 };
Texture2D texLuckyForLife = { 0 };

void init_graphics()
{
	texMassCash = LoadTexture("masscash.jpeg");
	texPowerball = LoadTexture("powerball.jpg");
	texLuckyForLife = LoadTexture("luckyforlife.png");
}

void free_graphics()
{
	if (IsTextureValid(texMassCash))
		UnloadTexture(texMassCash);

	if (IsTextureValid(texPowerball))
		UnloadTexture(texPowerball);

	if (IsTextureValid(texLuckyForLife))
		UnloadTexture(texLuckyForLife);
}

void set_render_size()
{
	render_size.x = GetRenderWidth();
	render_size.y = GetRenderHeight();
}

// immediate mode GUI
void render_choose_game()
{
	Vector2 origin = Vector2Zero();
	Rectangle src, dst;
	float sz = MIN(render_size.x / 5.0f, render_size.y / 5.0f);
	src.x = src.y = 0.0f;
	src.width = texMassCash.width;
	src.height = texMassCash.height;
	float s = MIN(sz / src.width, sz / src.height);
	dst.width = src.width * s;
	dst.height = src.height * s;
	dst.x = 10 + (sz - dst.width) / 2.0f;
	dst.y = 10 + (sz - dst.height) / 2.0f;
	DrawTexturePro(texMassCash, src, dst, origin, 0.0f, WHITE);
}

void render()
{
	ClearBackground(bkgnd);
	if (pgd == NULL)
		render_choose_game();
}
