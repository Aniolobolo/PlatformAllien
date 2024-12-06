// Bullet.cpp
#include "Bullet.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "EntityManager.h"
#include "Player.h"

Bullet::Bullet()
    : Entity(EntityType::BULLET),
    direction(1, 0),
    pbody(nullptr),
    texW(0),
    texH(0),
    texture(nullptr),
    texturePath("")  // Inicializar todas las variables miembro
{
    name = "bullet";
}

Bullet::~Bullet() {}

bool Bullet::Awake() {
    return true;
}

bool Bullet::Start() {
    // Inicializar texturas
    texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/player/bullet2.png");
    position.setX(static_cast<float>(parameters.attribute("x").as_int()));  // Conversión a float
    position.setY(static_cast<float>(parameters.attribute("y").as_int()));  // Conversión a float
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Cargar animaciones
    travel.LoadAnimations(parameters.child("animations").child("travel"));
    currentAnimation = &travel;

    pbody = Engine::GetInstance().physics.get()->CreateRectangle(static_cast<int>(position.getX()), static_cast<int>(position.getY()), 22, 10, bodyType::DYNAMIC);
    if (pbody == nullptr) {
        LOG("Error: PhysBody creation failed!");
        return false;
    }
    else {
        LOG("PhysBody created successfully at position: (%f, %f)", position.getX(), position.getY());
    }

    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    // Establecer tipo de colisión
    pbody->ctype = ColliderType::BULLET;

    return true;
}

bool Bullet::Update(float dt) {
    if (pbody == nullptr) {
        LOG("Error: PhysBody creation failed!");
        return false;  // O realizar alguna otra acción para manejar el error
    }

    b2Vec2 velocity = pbody->body->GetLinearVelocity();
    velocity.x = direction.getX() * 5.0f;  // Velocidad constante en la dirección de la bala
    pbody->body->SetLinearVelocity(velocity);

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(static_cast<float>(METERS_TO_PIXELS(pbodyPos.p.x)) - 12.0f);  // Conversión a float
    position.setY(static_cast<float>(METERS_TO_PIXELS(pbodyPos.p.y)) - 12.0f);  // Conversión a float

    // Dibujar al enemigo en la pantalla y actualizar su animación
    if (direction.getX() < 0) {
        // Si la dirección es hacia la izquierda, voltear el sprite horizontalmente
        Engine::GetInstance().render.get()->DrawTexture(texture, static_cast<int>(position.getX()), static_cast<int>(position.getY()), nullptr, SDL_FLIP_HORIZONTAL);
    }
    else {
        // Si la dirección es hacia la derecha, dibujar el sprite normalmente
        Engine::GetInstance().render.get()->DrawTexture(texture, static_cast<int>(position.getX()), static_cast<int>(position.getY()));
    }
    currentAnimation->Update();

    return true;
}

bool Bullet::CleanUp() {
    if (pbody != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
        pbody = nullptr;  // Para asegurarte de que no haya referencias colgando
    }
    return true;
}

void Bullet::SetPosition(Vector2D pos) {
    if (pbody == nullptr) {
        LOG("Error: Cannot set position, pbody is NULL.");
        return; // Salir si el pbody es NULL
    }

    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);  // Establecer la nueva posición física

    position = pos;  // Actualizar la posición en pantalla
}

Vector2D Bullet::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(static_cast<float>(METERS_TO_PIXELS(bodyPos.x)), static_cast<float>(METERS_TO_PIXELS(bodyPos.y)));  // Conversión a float
    return pos;
}

void Bullet::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
    case ColliderType::HAZARD:
    case ColliderType::CHECKPOINT:
    case ColliderType::ITEM:
        LOG("Collided with PLATFORM - DESTROY");
        isAlive = false;
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    }
}

void Bullet::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
    case ColliderType::HAZARD:
    case ColliderType::CHECKPOINT:
    case ColliderType::ITEM:
        LOG("Collided with hazard - DESTROY");
        break;
    }
}
