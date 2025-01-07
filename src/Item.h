#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

enum class ItemType {
	COLLECT,
	POWERUP
};

class Item : public Entity
{
public:

	Item(ItemType iType = ItemType::COLLECT);
	virtual ~Item();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:
	bool isPicked = false;

private:
	SDL_Texture* texture;
	const char* texturePath;
	pugi::xml_node parameters;
	int texW, texH;

	ItemType itemType;

	Animation* currentAnimation = nullptr;
	Animation coin;
	Animation powerup;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
