#define __LOTTERY_IMPLEMENTATION__
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "lottery.h"
#include "parser.h"

const Color bkgnd = { 0, 32, 32, 255 };
GameData_t gamedata = { 0 };

Texture2D texMassCash = { 0 };
Texture2D texPowerball = { 0 };
Vector2 v2zero;

void choose_game(char* gamename)
{
	if (strcmp(gamename, "masscash") == 0)
	{
		sprintf(gamedata.display_name, "Mass Cash");
		sprintf(gamedata.identifier, "mass_cash");
		sprintf(gamedata.first_draw, "1991-03-28");
		gamedata.cost = 1;
		gamedata.multiplier_cost = 0;
		gamedata.nBalls = 35;
		gamedata.nDraw = 5;
		gamedata.nBonus = 0;

		parse_game(gamename, &gamedata);
	}
	else
		memset(&gamedata, 0, sizeof(GameData_t));
}

void init_lottery()
{
	memset(&gamedata, 0, sizeof(GameData_t));
	texMassCash = LoadTexture("/home/ed/lottery/logos/mass_cash.png");
	texPowerball = LoadTexture("/home/ed/lottery/logos/powerball.png");
	atexit(free_lottery);
	v2zero = Vector2Zero();
}

void free_lottery()
{
	if (IsTextureValid(texMassCash))
		UnloadTexture(texMassCash);

	if (IsTextureValid(texPowerball))
		UnloadTexture(texPowerball);
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
	float height = (render_size.y - 30.0f) / 2.0f;
	float cx = render_size.x / 2.0f;
	float cy = render_size.y / 2.0f;

	if (texture_box(texMassCash, cx, cy - height / 2.0f - 10.0f, -1.0f, height, "masscash"))
		return;

	if (texture_box(texPowerball, cx, cy + height / 2.0f + 10.0f, -1.0f, height, "powerball"))
		return;
}

void render()
{
	ClearBackground(bkgnd);
	if (gamedata.display_name[0] == 0)
	{
		render_choose_game();
	}
	else
	{
		int szy = (int)floor(render_size.y / 25.0f);
		int szx = MeasureText(gamedata.display_name, szy);
		DrawText(gamedata.display_name, (render_size.x - szx) / 2, 0, szy, GREEN);
	}
}
