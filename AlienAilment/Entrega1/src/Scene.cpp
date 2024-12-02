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
	
	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item->position = Vector2D(200, 672);
	
	pugi::xml_node checkpoint = configParameters.child("entities").child("checkpoints").child("checkpoint");
	checkP = (Checkpoint*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
	checkP->SetParameters(checkpoint);

	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
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

	// Texture to highligh mouse position 
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Textures/props/goldCoin.png");

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
	Engine::GetInstance().render.get()->camera.x = 500 - player->position.getX();
	
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


	//Get mouse position and obtain the map coordinate
	Vector2D mousePos = Engine::GetInstance().input.get()->GetMousePosition();
	Vector2D mouseTile = Engine::GetInstance().map.get()->WorldToMap(mousePos.getX() - Engine::GetInstance().render.get()->camera.x,
		mousePos.getY() - Engine::GetInstance().render.get()->camera.y);


	//Render a texture where the mouse is over to highlight the tile, use the texture 'mouseTileTex'
	Vector2D highlightTile = Engine::GetInstance().map.get()->MapToWorld(mouseTile.getX(), mouseTile.getY());
	SDL_Rect rect = { 0,0,32,32 };
	Engine::GetInstance().render.get()->DrawTexture(mouseTileTex,
		highlightTile.getX(),
		highlightTile.getY(),
		&rect);

	// saves the tile pos for debugging purposes
	if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
		tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + "," + std::to_string((int)mouseTile.getY()) + "] ";
		once = true;
	}

	//If mouse button is pressed modify enemy position
	if (Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_DOWN) {
		if (enemyList[0]->isAlive) {
			enemyList[0]->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
			enemyList[0]->ResetPath();
		}
		
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

	if (checkP->hasSounded) {
		SaveState();
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
	Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int(),
		sceneNode.child("entities").child("player").attribute("y").as_int());
	player->SetPosition(playerPos);

	//enemies
	// ...

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