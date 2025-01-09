#include "Render.h"
#include "Engine.h"
#include "Input.h"
#include "Window.h"
#include "Log.h"
#include "SDL2/SDL_ttf.h"
#include "GuiManager.h"
#include "GuiControlButton.h"

#define VSYNC true

Render::Render() : Module(), camera{ 0, 0, 0, 0 }, renderer(nullptr), usingVsync(false), viewport{ 0, 0, 0, 0 }
{
    name = "render";
    background.r = 0;
    background.g = 0;
    background.b = 0;
    background.a = 0;
}

Render::~Render() {}

void Render::InitButtons()
{
    int buttonWidth = 200;
    int buttonHeight = 50;
    int startX = 100;
    int startY = 100;
    int spacing = 10;

    for (int i = 0; i < 5; ++i) {
        SDL_Rect rect = { startX, startY + i * (buttonHeight + spacing), buttonWidth, buttonHeight };
        buttons[i] = GuiControlButton(i, rect, "Button");
        buttons[i].onClick = [i]() {
            LOG("Button %d clicked!", i);
            };
    }
}

void Render::HandleHover(int mouseX, int mouseY)
{
    for (int i = 0; i < 5; ++i) {
        buttons[i].isHovered = (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
            mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h);
    }
}

void Render::HandleEvent(SDL_Event* event)
{
    for (int i = 0; i < 5; ++i) {
        buttons[i].HandleEvent(event);
    }
}

bool Render::Update(float dt)
{
    for (int i = 0; i < 5; ++i) {
        buttons[i].Update(dt);
    }
    return true;
}

bool Render::Draw()
{
    for (int i = 0; i < 5; ++i) {
        buttons[i].Draw(renderer);
    }
    return true;
}

bool Render::DrawText(const char* text, int x, int y, int w, int h) const
{
    SDL_Color color = { 255, 255, 255, 255 }; // Color blanco
    return DrawText(text, x, y, w, h);
}

bool Render::Start()
{
    LOG("render start");
    SDL_RenderGetViewport(renderer, &viewport);
    return true;
}

bool Render::PreUpdate()
{
    SDL_RenderClear(renderer);
    return true;
}

bool Render::PostUpdate()
{
    SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
    SDL_RenderPresent(renderer);
    return true;
}

bool Render::CleanUp()
{
    LOG("Destroying SDL render");
    SDL_DestroyRenderer(renderer);
    return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
    background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
    SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
    SDL_RenderSetViewport(renderer, &viewport);
}

bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, SDL_RendererFlip flip_, float speed, double angle, int pivotX, int pivotY) const
{
    bool ret = true;
    int scale = Engine::GetInstance().window.get()->GetScale();

    SDL_Rect rect;
    rect.x = (int)(camera.x * speed) + x * scale;
    rect.y = (int)(camera.y * speed) + y * scale;

    if (section != NULL)
    {
        rect.w = section->w;
        rect.h = section->h;
    }
    else
    {
        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    }

    rect.w *= scale;
    rect.h *= scale;

    SDL_Point* p = NULL;
    SDL_Point pivot;

    if (pivotX != INT_MAX && pivotY != INT_MAX)
    {
        pivot.x = pivotX;
        pivot.y = pivotY;
        p = &pivot;
    }

    if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip_) != 0)
    {
        LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
    bool ret = true;
    int scale = Engine::GetInstance().window.get()->GetScale();

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    SDL_Rect rec(rect);
    if (use_camera)
    {
        rec.x = (int)(camera.x + rect.x * scale);
        rec.y = (int)(camera.y + rect.y * scale);
        rec.w *= scale;
        rec.h *= scale;
    }

    int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

    if (result != 0)
    {
        LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
    bool ret = true;
    int scale = Engine::GetInstance().window.get()->GetScale();

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    int result = -1;

    if (use_camera)
        result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
    else
        result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

    if (result != 0)
    {
        LOG("Cannot draw line to screen. SDL_RenderDrawLine error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}
