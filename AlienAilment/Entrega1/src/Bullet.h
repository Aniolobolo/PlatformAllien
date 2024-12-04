#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Bullet : public Entity
{
public:

	Bullet();
	virtual ~Bullet();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool isAlive = true;
	int distCounter = 0;

	SDL_RendererFlip hflip = SDL_FLIP_NONE;
public:

private:
	bool flipSprite = false;

	SDL_Texture* texture;
	const char* texturePath;

	pugi::xml_node parameters;
	int texW, texH;

	float speed = 5.0f;

	Animation* currentAnimation = nullptr;
	Animation travel;
	PhysBody* pbody;
};
