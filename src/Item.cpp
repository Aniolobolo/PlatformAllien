#include "Item.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

// Constructor actualizado
Item::Item(ItemType iType) : Entity(EntityType::ITEM), itemType(iType)
{
    name = "item";
}

Item::~Item() {}

bool Item::Awake() {
    return true;
}

bool Item::Start() {
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    coin.LoadAnimations(parameters.child("animations").child("coin"));
    powerup.LoadAnimations(parameters.child("animations").child("powerup"));

    // L08 TODO 4: Add a physics to an item - initialize the physics body
    Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 4, (int)position.getY() + texH / 4, 16, bodyType::STATIC);

    // L08 TODO 7: Assign collider type
    if (itemType == ItemType::COLLECT) {
        currentAnimation = &coin;
        pbody->ctype = ColliderType::ITEM;
    }
    else if (itemType == ItemType::POWERUP) {
        currentAnimation = &powerup;
        pbody->ctype = ColliderType::POWERUP;
    }

    pbody->listener = this;

    return true;
}

bool Item::Update(float dt)
{
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 4);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 4);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();

    return true;
}

bool Item::CleanUp()
{
    if (pbody != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
        Engine::GetInstance().textures.get()->UnLoad(texture);
        pbody = nullptr;  // Para asegurarte de que no haya referencias colgando
    }
    return true;
}

void Item::OnCollision(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
    case ColliderType::PLAYER:
        LOG("Item collided with player");
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);

        break;
	}
}

void Item::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with player");
		break;
	}
}
