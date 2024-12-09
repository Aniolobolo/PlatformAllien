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

Scene::Scene() : Module(), bgMusic(0), checkP(nullptr), player(nullptr), areControlsVisible(false), hasReachedCheckpoint(false), controls(nullptr)
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

    // Crear un item
    Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
    item->position = Vector2D(100, 500);

	// Crear un checkpoint
    pugi::xml_node checkpoint = configParameters.child("entities").child("checkpoints").child("checkpoint");
    checkP = (Checkpoint*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
    checkP->SetParameters(checkpoint);
    
	// Crear enemigos
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
    // Si el jugador no está en respawn y está dentro de los intervalos, mueve la cámara
    Engine::GetInstance().render.get()->camera.x = 500 - player->position.getX();

	// Mostrar el menú de controles
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

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
        player->SetPosition(Vector2D(180, 500));

	// Comprobar si el jugador ha llegado al checkpoint
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

    if (!result) {
        LOG("Could not load file. Pugi error: %s", result.description());
        return;
    }

    pugi::xml_node sceneNode = loadFile.child("config").child("scene");

    // Cargar la posición del jugador
    pugi::xml_node playerNode = sceneNode.child("entities").child("player");
    if (playerNode) {
        Vector2D playerPos(
            static_cast<float>(playerNode.attribute("x").as_int() - 32),
            static_cast<float>(playerNode.attribute("y").as_int() - 32)
        );
        player->SetPosition(playerPos);
    }

	// AQUI HEMOS INTENTADO IMPLEMENTAR EL LOAD PARA LOS ENEMIGOS. COMO TAL FUNCIONA PERO CUANDO EL ENEMIGO SE MUERE Y SE VUELVE A CARGAR, NO FUNCIONA Y EL CODIGO PETA.

    //// Cargar enemigos
    //int i = 0;
    //for (pugi::xml_node enemyNode = sceneNode.child("entities").child("enemies").child("enemy");
    //    enemyNode;
    //    enemyNode = enemyNode.next_sibling("enemy")) {
    //    float x = static_cast<float>(enemyNode.attribute("x").as_int() - 32);
    //    float y = static_cast<float>(enemyNode.attribute("y").as_int() - 32);
    //    bool isAlive = enemyNode.attribute("alive").as_bool();

    //    if (isAlive && i < enemyList.size()) { // Actualizar enemigo existente solo si está vivo
    //        enemyList[i]->SetPosition(Vector2D(x, y));
    //        enemyList[i]->SetAlive();
    //        i++;
    //    }
    //}

    //// Cargar enemigos en el suelo
    //i = 0;
    //for (pugi::xml_node enemyFNode = sceneNode.child("entities").child("enemies").child("enemyFloor");
    //    enemyFNode;
    //    enemyFNode = enemyFNode.next_sibling("enemyFloor")) {
    //    float x = static_cast<float>(enemyFNode.attribute("x").as_int() - 32);
    //    float y = static_cast<float>(enemyFNode.attribute("y").as_int() - 32);
    //    bool isAlive = enemyFNode.attribute("alive").as_bool();

    //    if (isAlive && i < enemyFList.size()) { // Actualizar enemigo existente solo si está vivo
    //        enemyFList[i]->SetPosition(Vector2D(x, y));
    //        enemyFList[i]->SetAlive();
    //        i++;
    //    }
    //}
}


void Scene::SaveState() {
    pugi::xml_document saveFile;
    pugi::xml_parse_result result = saveFile.load_file("config.xml");

    if (!result) {
        LOG("Could not load file. Pugi error: %s", result.description());
        return;
    }

    pugi::xml_node sceneNode = saveFile.child("config").child("scene");

    // Guardar la posición del jugador
    pugi::xml_node playerNode = sceneNode.child("entities").child("player");
    if (playerNode) {
        playerNode.attribute("x").set_value(player->GetPosition().getX());
        playerNode.attribute("y").set_value(player->GetPosition().getY());
    }

    // AQUI HEMOS INTENTADO IMPLEMENTAR EL SAVE PARA LOS ENEMIGOS. COMO TAL FUNCIONA PERO COMO EL LOAD NO FUNCIONABA PUES AL FINAL LO HEMOS DESCARTADO.

    //// Guardar enemigos
    //pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
    //for (Enemy* enemy : enemyList) {
    //    pugi::xml_node enemyNode = enemiesNode.append_child("enemy");
    //    enemyNode.append_attribute("x").set_value(enemy->GetPosition().getX());
    //    enemyNode.append_attribute("y").set_value(enemy->GetPosition().getY());
    //    enemyNode.append_attribute("alive").set_value(enemy->isAlive());
    //}

    //// Guardar enemigos en el suelo
    //for (EnemyFloor* enemyF : enemyFList) {
    //    pugi::xml_node enemyFNode = enemiesNode.append_child("enemyFloor");
    //    enemyFNode.append_attribute("x").set_value(enemyF->GetPosition().getX());
    //    enemyFNode.append_attribute("y").set_value(enemyF->GetPosition().getY());
    //    enemyFNode.append_attribute("alive").set_value(enemyF->isAlive());
    //}

    // Guardar las modificaciones en el archivo XML
    saveFile.save_file("config.xml");
}
