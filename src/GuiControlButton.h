#pragma once
#include "SDL2/SDL.h"
#include <functional>

class GuiControlButton
{

	GuiControlButton(int id, SDL_Rect rect, const char* text);
	void HandleEvent(SDL_Event* event);
	void Update(float dt);
	void Draw(SDL_Renderer* renderer);
	int id;
	SDL_Rect rect;
	std::function<void()> onClick;
	bool isHovered;

};


