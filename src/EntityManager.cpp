#include "EntityManager.h"
#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Log.h"
#include "Item.h"
#include "Enemy.h"
#include "Checkpoint.h"
#include "Bullet.h"
#include "EnemyFloor.h"
#include "Boss.h"

EntityManager::EntityManager() : Module()
{
    name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager()
{
    CleanUp();
}

// Called before render is available
bool EntityManager::Awake()
{
    LOG("Loading Entity Manager");
    bool ret = true;

    // Iterates over the entities and calls Awake
    for (const auto& entity : entities)
    {
        if (entity == nullptr || !entity->active) continue;
        ret = entity->Awake();
    }

    return ret;
}

bool EntityManager::Start()
{
    bool ret = true;

    // Iterates over the entities and calls Start
    for (const auto& entity : entities)
    {
        if (entity == nullptr || !entity->active) continue;
        ret = entity->Start();
    }

    return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
    bool ret = true;

    for (auto& entity : entities)
    {
        if (entity != nullptr)
        {
            if (entity->active)
                ret = entity->CleanUp();
            delete entity;
            entity = nullptr; // Prevent dangling pointer
        }
    }

    entities.clear();

    return ret;
}

Entity* EntityManager::CreateEntity(EntityType type)
{
    Entity* entity = nullptr;

    // Instantiate entity according to the type and add it to the list of entities
    switch (type)
    {
    case EntityType::PLAYER:
        entity = new Player();
        break;
    case EntityType::ITEM:
        entity = new Item();
        break;
    case EntityType::ENEMY:
        entity = new Enemy();
        break;
    case EntityType::ENEMYFLOOR:
        entity = new EnemyFloor();
        break;
    case EntityType::BOSS:
        entity = new Boss();
        break;
    case EntityType::CHECKPOINT:
        entity = new Checkpoint();
        break;
    case EntityType::BULLET:
        entity = new Bullet();
        break;
    default:
        break;
    }

    if (entity != nullptr)
    {
        entity->active = true; // Ensure the entity is active by default
        entities.push_back(entity);
    }

    return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end())
    {
        if (*it != nullptr)
        {
            (*it)->CleanUp();
            delete* it;
            *it = nullptr; // Prevent dangling pointer
        }
        entities.erase(it);
    }
}

void EntityManager::AddEntity(Entity* entity)
{
    if (entity != nullptr)
    {
        entity->active = true; // Ensure the added entity is active
        entities.push_back(entity);
    }
}

bool EntityManager::Update(float dt)
{
    bool ret = true;
    std::vector<Entity*> entitiesToRemove;

    for (auto& entity : entities)
    {
        if (entity == nullptr) continue;

        // Update only active entities
        if (entity->active)
        {
            ret = entity->Update(dt);
        }
        else
        {
            entitiesToRemove.push_back(entity);
        }
    }

    // Remove entities after the iteration
    for (auto& entity : entitiesToRemove)
    {
        DestroyEntity(entity);
    }

    return ret;
}
