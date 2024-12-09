#include "Enemy.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "EntityManager.h"

Enemy::Enemy() : Entity(EntityType::ENEMY), pathfinding(nullptr), pbody(nullptr)
{

}

Enemy::~Enemy() {
	if (pathfinding != nullptr) {
		delete pathfinding;
	}
}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {

	// Inicializar texturas
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	// Cargar animaciones
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;

	// Agregar f�sica al objeto - inicializar el cuerpo f�sico
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 4, (int)position.getY() + texH / 4, texH / 4, bodyType::DYNAMIC);

	// Verificar si pbody se inicializ� correctamente
	if (pbody == nullptr) {
		LOG("Error al crear el cuerpo f�sico del enemigo");
		return false;
	}

	// Asignar tipo de colisionador
	pbody->ctype = ColliderType::ENEMY;
	pbody->listener = this;

	// Configurar la gravedad del cuerpo
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	deathSfx = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/enemy_death.wav");

	// Establecer la velocidad inicial a cero para evitar movimiento no deseado
	pbody->body->SetLinearVelocity(b2Vec2(0, 0));

	// Inicializar pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	isalive = true;

	return true;
}

void Enemy::MoveTowardsTargetTile(float dt)
{
	// Asegurarse de que hay suficientes tiles en el camino para calcular el movimiento
	if (pathfinding == nullptr || pathfinding->pathTiles.size() < 2) return;

	// Paso 1: Verificar si el path tiene m�s de 20 tiles
	if (pathfinding->pathTiles.size() > 20) {
		// Detener el movimiento si el path tiene m�s de 20 tiles
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		return;
	}

	// Obtener el pen�ltimo tile (el objetivo inmediato)
	auto it = pathfinding->pathTiles.end();
	Vector2D targetTile = *(--(--it)); // Pen�ltimo tile
	Vector2D targetWorldPos = Engine::GetInstance().map.get()->MapToWorld(targetTile.getX(), targetTile.getY());

	// Calcular la direcci�n hacia el pen�ltimo tile
	Vector2D enemyPos = GetPosition();
	Vector2D direction = targetWorldPos + Vector2D(16, 16) - enemyPos;

	// Calcular distancia al objetivo
	float distance = direction.Length();

	// Normalizar la direcci�n para calcular la velocidad
	Vector2D velocity(0, 0);
	if (distance > 0.5f) { // Si la distancia es mayor a un umbral, moverse
		direction.Normalize();
		velocity = direction * 100.0f; // Ajustar la velocidad seg�n necesidades
	}

	// Aplicar la velocidad al cuerpo f�sico del enemigo
	b2Vec2 velocityVec = b2Vec2(PIXEL_TO_METERS(velocity.getX()), PIXEL_TO_METERS(velocity.getY()));
	pbody->body->SetLinearVelocity(velocityVec);
}

bool Enemy::Update(float dt)
{
	// Paso 1: Recalcular el camino hacia el jugador
	ResetPath(); // Restablecer el camino antes de calcular uno nuevo

	// Propagar A* hasta que se obtenga un camino v�lido
	while (pathfinding->pathTiles.empty()) {
		pathfinding->PropagateAStar(SQUARED);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN) {
		draw = !draw;
	}

	if (draw) {
		pathfinding->DrawPath();
	}

	// Paso 3: Mover al enemigo hacia el pen�ltimo tile
	MoveTowardsTargetTile(dt);

	// Obtener la velocidad actual del cuerpo f�sico
	float velocityX = pbody->body->GetLinearVelocity().x;

	// Actualizar el estado de flip basado en la direcci�n del movimiento
	if (velocityX < -0.1f) { // Movimiento hacia la izquierda
		flipSprite = true;
		hflip = SDL_FLIP_HORIZONTAL;
	}
	else if (velocityX > 0.1f) { // Movimiento hacia la derecha
		flipSprite = false;
		hflip = SDL_FLIP_NONE;
	}

	// Actualizar la posici�n f�sica del enemigo basada en la simulaci�n de f�sica
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Dibujar al enemigo en la pantalla y actualizar su animaci�n
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), hflip);
	currentAnimation->Update();

	return true;
}

bool Enemy::CleanUp()
{
	if (pbody != nullptr) {
		Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	}
	return true;
}

void Enemy::CreateEnemyAtPosition(Vector2D position) {
	Enemy* newEnemy = new Enemy();
	newEnemy->SetPosition(position);
	Engine::GetInstance().entityManager.get()->AddEntity(newEnemy);
}

void Enemy::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 4);
	pos.setY(pos.getY() + texH / 4);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	if (pbody != nullptr) {
		pbody->body->SetTransform(bodyPos, 0);
	}
}

Vector2D Enemy::GetPosition() {
	if (!isalive) {
		return Vector2D(0, 0);
	}
	if (pbody != nullptr) {
		b2Vec2 bodyPos = pbody->body->GetTransform().p;
		Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
		return pos;
	}
	return Vector2D(0, 0);
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	if (pathfinding != nullptr) {
		pathfinding->ResetPath(tilePos);
	}
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::BULLET:
	case ColliderType::VOID:
		LOG("Collided with hazard - DESTROY");
		SetDead();
		Engine::GetInstance().audio.get()->PlayFx(deathSfx);
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
	}
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::BULLET:
		LOG("Collision hazard");
		break;
	}
}

void Enemy::SetAlive() {
	isalive = true;
	parameters.attribute("alive").set_value(true);
}

void Enemy::SetDead() {
	isalive = false;
	parameters.attribute("alive").set_value(false);
}