#include "GuiControlButton.h"
#include "Log.h"

GuiControlButton::GuiControlButton(int id, SDL_Rect rect, const char* text)
	: id(id), rect(rect), isHovered(false)
{
	// Initialize other members if necessary
}

void GuiControlButton::HandleEvent(SDL_Event* event)
{
	// Handle the event
}

void GuiControlButton::Update(float dt)
{
	// Update the button state
}

void GuiControlButton::Draw(SDL_Renderer* renderer)
{
	// Draw the button
}

