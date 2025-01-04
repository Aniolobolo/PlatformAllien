#pragma once

#pragma once

#include <vector>
#include "Module.h"
#include "Entity.h"

class EntityManager : public Module
{
public:
    EntityManager();
    ~EntityManager();

    bool Awake();
    bool Start();
    bool CleanUp();
    bool Update(float dt);

    Entity* CreateEntity(EntityType type);
    void DestroyEntity(Entity* entity);
    void AddEntity(Entity* entity);

private:
    std::vector<Entity*> entities;
};
