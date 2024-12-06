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
	void SetDirection(const Vector2D& direction);


	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool isAlive = true;
	int distCounter = 0;

	SDL_RendererFlip hflip = SDL_FLIP_NONE;
public:
	SDL_Texture* texture;

private:
	bool flipSprite = false;

	
	const char* texturePath;

	pugi::xml_node parameters;
	int texW, texH;

	Animation* currentAnimation = nullptr;
	Animation travel;
	PhysBody* pbody;
};
