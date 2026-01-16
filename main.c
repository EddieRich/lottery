#include "lottery.h"

const char* title = "Lottery";
int winx, winy, winw, winh;

// return true to exit
bool event(void)
{
	if (WindowShouldClose() || IsKeyPressed(KEY_Q))
		return true;

	if (IsKeyPressed(KEY_F11))	// pressed and released
	{
		if (IsWindowFullscreen())
		{
			ToggleFullscreen();
			SetWindowSize(winw, winh);
			SetWindowPosition(winx, winy);
		}
		else
		{
			Vector2 pos = GetWindowPosition();
			winx = (int)pos.x;
			winy = (int)pos.y;
			winw = GetScreenWidth();
			winh = GetScreenHeight();
			int monitor = GetCurrentMonitor();
			SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
			ToggleFullscreen();
		}
	}

	if (IsWindowResized())
		set_render_size();

	return false;
}

int main(void)
{
	SetTraceLogLevel(LOG_NONE);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(10);
	InitWindow(800, 600, title);

	init_lottery();

	if (!IsWindowReady())
		return 1;

	set_render_size();


	render();

	while (!event())
	{
		BeginDrawing();
		render();
		EndDrawing();
	}

	free_lottery();
	return 0;
}
