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
{}

// Called before render is available
bool EntityManager::Awake()
{
	LOG("Loading Entity Manager");
	bool ret = true;

	//Iterates over the entities and calls the Awake
	for (const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Awake();
	}

	return ret;

}

bool EntityManager::Start() {

	bool ret = true;

	//Iterates over the entities and calls Start
	for (const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Start();
	}

	return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	bool ret = true;

	for (const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->CleanUp();
	}

	entities.clear();

	return ret;
}

Entity* EntityManager::CreateEntity(EntityType type)
{
	Entity* entity = nullptr;

	//L04: TODO 3a: Instantiate entity according to the type and add the new entity to the list of Entities
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

	entities.push_back(entity);

	return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
	auto it = std::find(entities.begin(), entities.end(), entity);
	if (it != entities.end())
	{
		(*it)->CleanUp();
		delete* it; // Liberar la memoria asignada
		entities.erase(it); // Eliminar la entidad de la lista
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if (entity != nullptr) entities.push_back(entity);
}

bool EntityManager::Update(float dt)
{
	bool ret = true;
	std::vector<Entity*> entitiesToRemove;

	for (auto entity : entities)
	{
		// Actualizar solo entidades activas
		if (entity->active)
		{
			ret = entity->Update(dt);
		}
		else
		{
			entitiesToRemove.push_back(entity);
		}
	}

	// Eliminar entidades despu�s de la iteraci�n
	for (auto entity : entitiesToRemove)
	{
		DestroyEntity(entity);
	}

	return ret;
}