#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Boss : public Entity
{
public:

	Boss();
	virtual ~Boss();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();
	Vector2D GetDirection() const;

	void ResetPath();
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);
	void MoveTowardsTargetTile(float dt);
	bool isAlive() const { return isalive; }
	void SetAlive();
	void SetDead();
	void CreateEnemyAtPosition(Vector2D position);

	int distCounter = 0;

	SDL_RendererFlip hflip = SDL_FLIP_NONE;
public:

private:
	Vector2D lastEnemyTile;
	Vector2D lastPlayerTile;
	std::vector<Vector2D> pathTiles;  // Almacena la ruta de tiles
	int currentPathIndex = 0;  // Índice actual en la ruta
	float ENEMY_SPEED = 15.0f;
	int health = 50;
	int maxHealth = 50;

	bool isShooting = false;
	bool isalive = true;
	bool isDying = false;
	bool flipSprite = false;
	bool draw = false;

	int deathSfx;
	int shootFxId;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;

	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation move;
	Animation shoot;
	Animation shootD;
	Animation die;

	PhysBody* pbody;
	Pathfinding* pathfinding;
};
