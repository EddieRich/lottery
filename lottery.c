#define __LOTTERY_IMPLEMENTATION__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lottery.h"
#include "parser.h"

const Color bkgnd = { 0, 32, 32, 255 };
GameMatrix_t gm = { 0 };

Texture2D texMassCash = { 0 };
Texture2D texPowerball = { 0 };
Texture2D texLuckyForLife = { 0 };
Vector2 v2zero;

void choose_game(char* gamename)
{
	if (strcmp(gamename, "masscash") == 0)
	{
		sprintf(gm.display_name, "Mass Cash");
		sprintf(gm.identifier, "mass_cash");
		sprintf(gm.first_draw, "1991-03-28");
		gm.cost = 1;
		gm.multiplier_cost = 0;
		gm.nBalls = 35;
		gm.nDraw = 5;
		gm.nBonus = 0;

		parse(gamename, &gm);
	}
	else
		memset(&gm, 0, sizeof(GameMatrix_t));
}

void init_lottery()
{
	memset(&gm, 0, sizeof(GameMatrix_t));
	texMassCash = LoadTexture("/home/ed/lottery/logos/mass_cash.png");
	texPowerball = LoadTexture("/home/ed/lottery/logos/powerball.png");
	texLuckyForLife = LoadTexture("/home/ed/lottery/logos/lucky_for_life.png");
	atexit(free_lottery);
	v2zero = Vector2Zero();
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

bool texture_box(Texture2D tex, float cx, float cy, float width, float height, char* name)
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

	if (texture_box(texMassCash, cx, cy - height - 10.0f, -1.0f, height, "masscash"))
		return;

	if (texture_box(texPowerball, cx, cy, -1.0f, height, "powerball"))
		return;

	if (texture_box(texLuckyForLife, cx, cy + height + 10.0f, -1.0f, height, "luckyforlife"))
		return;
}

void render()
{
	ClearBackground(bkgnd);
	if (gm.display_name[0] == 0)
	{
		render_choose_game();
	}
	else
	{
		int szy = render_size.y / 20;
		int szx = MeasureText(gm.display_name, szy);
		DrawText(gm.display_name, (render_size.x - szx) / 2, 0, szy, GREEN);
	}
}
