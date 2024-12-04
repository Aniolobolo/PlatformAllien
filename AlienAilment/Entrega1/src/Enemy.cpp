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

Enemy::Enemy() : Entity(EntityType::ENEMY)
{

}

Enemy::~Enemy() {
	delete pathfinding;
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

	// Agregar física al objeto - inicializar el cuerpo físico
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 4, (int)position.getY() + texH / 4, texH / 4, bodyType::DYNAMIC);

	// Asignar tipo de colisionador
	pbody->ctype = ColliderType::ENEMY;
	pbody->listener = this;

	// Configurar la gravedad del cuerpo
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Establecer la velocidad inicial a cero para evitar movimiento no deseado
	pbody->body->SetLinearVelocity(b2Vec2(0, 0));

	// Inicializar pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	isAlive = true;

	return true;
}

void Enemy::MoveTowardsTargetTile(float dt)
{
	// Asegurarse de que hay suficientes tiles en el camino para calcular el movimiento
	if (pathfinding->pathTiles.size() < 2) return;

	// Paso 1: Verificar si el path tiene más de 20 tiles
	if (pathfinding->pathTiles.size() > 20) {
		// Detener el movimiento si el path tiene más de 20 tiles
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		return;
	}

	// Obtener el penúltimo tile (el objetivo inmediato)
	auto it = pathfinding->pathTiles.end();
	Vector2D targetTile = *(--(--it)); // Penúltimo tile
	Vector2D targetWorldPos = Engine::GetInstance().map.get()->MapToWorld(targetTile.getX(), targetTile.getY());

	// Calcular la dirección hacia el penúltimo tile
	Vector2D enemyPos = GetPosition();
	Vector2D direction = targetWorldPos + Vector2D(16, 16) - enemyPos;

	// Calcular distancia al objetivo
	float distance = direction.Length();

	// Normalizar la dirección para calcular la velocidad
	Vector2D velocity(0, 0);
	if (distance > 0.5f) { // Si la distancia es mayor a un umbral, moverse
		direction.Normalize();
		velocity = direction * 100.0f; // Ajustar la velocidad según necesidades
	}

	// Aplicar la velocidad al cuerpo físico del enemigo
	b2Vec2 velocityVec = b2Vec2(PIXEL_TO_METERS(velocity.getX()), PIXEL_TO_METERS(velocity.getY()));
	pbody->body->SetLinearVelocity(velocityVec);
}

bool Enemy::Update(float dt)
{
	// Paso 1: Recalcular el camino hacia el jugador
	ResetPath(); // Restablecer el camino antes de calcular uno nuevo

	// Propagar A* hasta que se obtenga un camino válido
	while (pathfinding->pathTiles.empty()) {
		pathfinding->PropagateAStar(SQUARED);
	}

	// Paso 2: Dibujar el camino calculado para debug
	pathfinding->DrawPath();

	// Paso 3: Mover al enemigo hacia el penúltimo tile
	MoveTowardsTargetTile(dt);

	// Obtener la velocidad actual del cuerpo físico
	float velocityX = pbody->body->GetLinearVelocity().x;

	// Actualizar el estado de flip basado en la dirección del movimiento
	if (velocityX < -0.1f) { // Movimiento hacia la izquierda
		flipSprite = true;
		hflip = SDL_FLIP_HORIZONTAL;
	}
	else if (velocityX > 0.1f) { // Movimiento hacia la derecha
		flipSprite = false;
		hflip = SDL_FLIP_NONE;
	}

	// Actualizar la posición física del enemigo basada en la simulación de física
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Dibujar al enemigo en la pantalla y actualizar su animación
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), hflip);
	currentAnimation->Update();

	return true;
}


bool Enemy::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void Enemy::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 4);
	pos.setY(pos.getY() + texH / 4);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Enemy::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with hazard - DESTROY");
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		isAlive = false;
		break;
	}
}
void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision hazard");
		isAlive = false;
		break;
	}
}