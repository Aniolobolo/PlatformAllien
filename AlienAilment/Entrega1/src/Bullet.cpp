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

Bullet::Bullet() : Entity(EntityType::BULLET)
{
	name = "bullet";
}

Bullet::~Bullet() {}

bool Bullet::Awake() {
	return true;
}

bool Bullet::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/player/bullet2.png");
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();


	// Cargar animaciones
	travel.LoadAnimations(parameters.child("animations").child("travel"));
	currentAnimation = &travel;

	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), 22, 10, bodyType::DYNAMIC);
	if (pbody == nullptr) {
		LOG("Error: PhysBody creation failed!");
		return false; 
	}
	else {
		LOG("PhysBody created successfully at position: (%f, %f)", position.getX(), position.getY());
	}

	pbody->listener = this;
	pbody->body->SetGravityScale(0);

	// Establecer tipo de colisi�n
	pbody->ctype = ColliderType::BULLET;




	return true;
}

bool Bullet::Update(float dt)
{
	if (pbody == nullptr) {
		LOG("Error: PhysBody creation failed!");
		return false;  // O realizar alguna otra acci�n para manejar el error
	}
	
	b2Vec2 velocity = pbody->body->GetLinearVelocity();
	velocity.x = 3.0f;  // Velocidad constante hacia la derecha
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - 12);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - 12);

	// Dibujar al enemigo en la pantalla y actualizar su animaci�n
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());
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


// Bullet.cpp
void Bullet::SetPosition(Vector2D pos) {
	if (pbody == nullptr) {
		LOG("Error: Cannot set position, pbody is NULL.");
		return; // Salir si el pbody es NULL
	}

	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);  // Establecer la nueva posici�n f�sica

	position = pos;  // Actualizar la posici�n en pantalla
}



Vector2D Bullet::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Bullet::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collided with PLATFORM - DESTROY");
		isAlive = false;
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
	case ColliderType::HAZARD:
		LOG("Collided with PLATFORM - DESTROY");
		isAlive = false;
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
	case ColliderType::CHECKPOINT:
		LOG("Collided with PLATFORM - DESTROY");
		isAlive = false;
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
	case ColliderType::ITEM:
		LOG("Collided with PLATFORM - DESTROY");
		isAlive = false;
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
	}
}
void Bullet::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision hazard");
		break;
	case ColliderType::HAZARD:
		LOG("Collided with hazard - DESTROY");
		break;
	case ColliderType::CHECKPOINT:
		LOG("Collided with hazard - DESTROY");
		break;
	case ColliderType::ITEM:
		LOG("Collided with hazard - DESTROY");
		break;

	}
}