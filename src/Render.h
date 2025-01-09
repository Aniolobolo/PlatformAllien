#ifndef __RENDER_H__
#define __RENDER_H__

#include "Module.h"
#include "SDL2/SDL.h"
#include "GuiControlButton.h"

class Render : public Module
{
public:
    Render();
    virtual ~Render();

    bool Awake(pugi::xml_node&);
    bool Start();
    bool PreUpdate();
    bool Update(float dt);
    bool PostUpdate();
    bool CleanUp();

    void InitButtons();
    void HandleHover(int mouseX, int mouseY);
    void HandleEvent(SDL_Event* event);
    bool Draw();
    bool DrawText(const char* text, int x, int y, int w, int h) const;
    bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, SDL_RendererFlip flip_, float speed, double angle, int pivotX, int pivotY) const;
    bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const;
    bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const;

    void SetBackgroundColor(SDL_Color color);
    void SetViewPort(const SDL_Rect& rect);
    void ResetViewPort();
    SDL_Renderer* renderer;
    SDL_Rect camera;
    SDL_Rect viewport;
    SDL_Color background;
    bool usingVsync;
    GuiControlButton buttons[5];
};

#endif // __RENDER_H__