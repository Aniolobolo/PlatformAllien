#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"


Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	//L04: TODO 3b: Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
	
	Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item->position = Vector2D(100, 500);
	
	pugi::xml_node checkpoint = configParameters.child("entities").child("checkpoints").child("checkpoint");
	checkP = (Checkpoint*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
	checkP->SetParameters(checkpoint);

	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemyFloor"); enemyNode; enemyNode = enemyNode.next_sibling("enemyFloor"))
	{
		EnemyFloor* enemyF = (EnemyFloor*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMYFLOOR);
		enemyF->SetParameters(enemyNode);
		enemyFList.push_back(enemyF);
	}

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());
	controls = Engine::GetInstance().textures->Load("Assets/Textures/Help.png");

	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());

	bgMusic = Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/music.ogg", 0);
	int musicVolume = 40;
	Mix_VolumeMusic(musicVolume);

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	// Si est� dentro de los intervalos, mueve la c�mara
	if (player->position.getX() >= 525 && player->position.getX() <= 3370) {
		Engine::GetInstance().render.get()->camera.x = 500 - player->position.getX();
	}
	else if (!player->isDead) {
		Engine::GetInstance().render.get()->camera.x = 500 - player->position.getX();
	}
	
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) {
		areControlsVisible = !areControlsVisible;
	}
	
	if (areControlsVisible && controls != nullptr) {
		int width, height;
		Engine::GetInstance().textures->GetSize(controls, width, height);
		int windowWidth, windowHeight;
		Engine::GetInstance().window->GetWindowSize(windowWidth, windowHeight);

		SDL_Rect dstRect = { windowWidth - width - 10, 10, width, height };


		SDL_RenderCopy(Engine::GetInstance().render->renderer, controls, nullptr, &dstRect);
	}

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		LoadState();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		SaveState();

	if (!hasReachedCheckpoint) {
		if (checkP->hasSounded) {
			SaveState();
			hasReachedCheckpoint = true;
		}
	}

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);
	if (controls != nullptr) {
		Engine::GetInstance().textures->UnLoad(controls);
		controls = nullptr;
	}
	return true;
}

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}

// L15 TODO 1: Implement the Load function
void Scene::LoadState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode = loadFile.child("config").child("scene");

	//Read XML and restore information

	//Player position
	Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int() - 32,
		sceneNode.child("entities").child("player").attribute("y").as_int() - 32);
	player->SetPosition(playerPos);



	//enemies

}

// L15 TODO 2: Implement the Save function
void Scene::SaveState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode = loadFile.child("config").child("scene");

	//Save info to XML 

	//Player position
	sceneNode.child("entities").child("player").attribute("x").set_value(player->GetPosition().getX());
	sceneNode.child("entities").child("player").attribute("y").set_value(player->GetPosition().getY());

	//enemies
	// ...

	//Saves the modifications to the XML 
	loadFile.save_file("config.xml");
}