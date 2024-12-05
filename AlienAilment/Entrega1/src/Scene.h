#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "EnemyFloor.h"
#include "Checkpoint.h"
#include "Bullet.h"
#include <vector>

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Return the player position
	Vector2D GetPlayerPosition();

	//L15 TODO 1: Implement the Load function
	void LoadState();
	//L15 TODO 2: Implement the Save function
	void SaveState();


public:
	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;

	}

private:
	SDL_Texture* img;
	bool areControlsVisible = false;
	SDL_Texture* controls = nullptr;
	int bgMusic;
	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	Checkpoint* checkP;
	SDL_Texture* mouseTileTex = nullptr;
	std::string tilePosDebug = "[0,0]";
	bool once = false;
	std::vector<Enemy*> enemyList;
	std::vector<EnemyFloor*> enemyFList;
	std::vector<Bullet*> bulletList;
	bool hasReachedCheckpoint = false;
};