#define __LOTTERY_IMPLEMENTATION__
#include <stdlib.h>
#include <string.h>
#include "lottery.h"

const Color bkgnd = { 0, 32, 32, 255 };
const GameMatrix_t* pgm = NULL;

Texture2D texMassCash = { 0 };
Texture2D texPowerball = { 0 };
Texture2D texLuckyForLife = { 0 };
Vector2 v2zero;

void choose_game(const char* gamename)
{
	if (strcmp(gamename, "FOO") == 0)
		pgm = NULL;
}

void init_lottery()
{
	texMassCash = LoadTexture("/home/ed/lottery/logos/mass_cash.png");
	texPowerball = LoadTexture("/home/ed/lottery/logos/powerball.png");
	texLuckyForLife = LoadTexture("/home/ed/lottery/logos/lucky_for_life.png");
	atexit(free_lottery);
	v2zero = Vector2Zero();
	get_games_meta();
}

void free_lottery()
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

bool texture_box(Texture2D tex, float cx, float cy, float width, float height, const char* name)
{
	Rectangle src, dst;

	src.x = src.y = 0.0f;
	src.width = tex.width;
	src.height = tex.height;

	if (height < 0.0f)
		height = width * src.height / src.width;
	else if (width < 0.0f)
		width = height * src.width / src.height;

	dst.width = width;
	dst.height = height;
	dst.x = cx - dst.width / 2.0f;
	dst.y = cy - dst.height / 2.0f;
	DrawTexturePro(tex, src, dst, v2zero, 0.0f, WHITE);

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		if (CheckCollisionPointRec(GetMousePosition(), dst))
			choose_game(name);
	}

	return false;
}

void render_choose_game()
{
	float height = (render_size.y - 40.0f) / 3.0f;
	float cx = render_size.x / 2.0f;
	float cy = render_size.y / 2.0f;

	if (texture_box(texMassCash, cx, cy - height - 10.0f, -1.0f, height, "mass_cash"))
		return;

	if (texture_box(texPowerball, cx, cy, -1.0f, height, "powerball"))
		return;

	if (texture_box(texLuckyForLife, cx, cy + height + 10.0f, -1.0f, height, "lucky_for_life"))
		return;
}

void render()
{
	ClearBackground(bkgnd);
	if (pgm == NULL)
	{
		render_choose_game();
	}
}
