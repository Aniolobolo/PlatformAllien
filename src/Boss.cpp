#include "Boss.h"
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

Boss::Boss() : Entity(EntityType::BOSS), pathfinding(nullptr), pbody(nullptr) {}

Boss::~Boss() {
	if (pathfinding != nullptr) {
		delete pathfinding;
	}
}

bool Boss::Awake() {
	return true;
}

// configurar al enemy
bool Boss::Start() {
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	move.LoadAnimations(parameters.child("animations").child("move"));
	shoot.LoadAnimations(parameters.child("animations").child("shoot"));
	shootD.LoadAnimations(parameters.child("animations").child("shootdown"));
	die.LoadAnimations(parameters.child("animations").child("die"));
	currentAnimation = &idle;
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 4, (int)position.getY() + texH / 4, texH / 4, bodyType::DYNAMIC);
	if (pbody == nullptr) {
		LOG("Error al crear el cuerpo físico del enemigo");
		return false;
	}
	pbody->ctype = ColliderType::ENEMY;
	pbody->listener = this;
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	shootFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/shoot.wav");
	deathSfx = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/enemy_death.wav");

	pbody->body->SetLinearVelocity(b2Vec2(0, 0));
	pathfinding = new Pathfinding();
	ResetPath();
	isalive = true;
	isDying = false;
	return true;
}

void Boss::MoveTowardsTargetTile(float dt) {
	if (pathfinding == nullptr || pathfinding->pathTiles.size() < 2) return;

	//si el player está a más de 20 tiles de distancia, el enemigo se queda quieto
	if (pathfinding->pathTiles.size() > 20) {
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		return;
	}

	//coger el penúltimo tile de la lista de tiles para moverse hacia él
	auto it = pathfinding->pathTiles.end();
	Vector2D targetTile = *(--(--it));
	Vector2D targetWorldPos = Engine::GetInstance().map.get()->MapToWorld(targetTile.getX(), targetTile.getY());
	Vector2D enemyPos = GetPosition();
	Vector2D direction = targetWorldPos + Vector2D(16, 16) - enemyPos;
	float distance = direction.Length();
	Vector2D velocity(0, 0);
	if (distance > 0.5f) {
		direction.Normalize();
		velocity = direction * 100.0f;
	}
	b2Vec2 velocityVec = b2Vec2(PIXEL_TO_METERS(velocity.getX()), PIXEL_TO_METERS(velocity.getY()));
	pbody->body->SetLinearVelocity(velocityVec);
}

bool Boss::Update(float dt) {
	ResetPath();
	if (!isDying) {
		while (pathfinding->pathTiles.empty()) {
			pathfinding->PropagateAStar(SQUARED);
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN) {
			draw = !draw;
		}
		if (draw) {
			pathfinding->DrawPath();
		}
		MoveTowardsTargetTile(dt);
		float velocityX = pbody->body->GetLinearVelocity().x;
		if (velocityX < -0.1f) {
			flipSprite = true;
			hflip = SDL_FLIP_HORIZONTAL;
		}
		else if (velocityX > 0.1f) {
			flipSprite = false;
			hflip = SDL_FLIP_NONE;
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_I) == KEY_REPEAT && !isShooting && currentAnimation == &idle) {
			isShooting = true;
			currentAnimation = &shoot;
			Vector2D bulletPosition = GetPosition();
			bulletPosition.setX(bulletPosition.getX() + (GetDirection().getX() * 28));
			Bullet* bullet = new Bullet(BulletType::BOSSH);
			bullet->SetDirection(GetDirection());
			bullet->SetParameters(Engine::GetInstance().scene.get()->configParameters);
			bullet->texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/enemies/bossBullet.png");
			Engine::GetInstance().entityManager.get()->AddEntity(bullet);
			bullet->Start();
			bullet->SetPosition(bulletPosition);
			Engine::GetInstance().audio.get()->PlayFx(shootFxId);
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_O) == KEY_REPEAT && !isShooting && currentAnimation == &idle) {
			isShooting = true;
			currentAnimation = &shootD;
			Vector2D bulletPosition = GetPosition();
			bulletPosition.setY(bulletPosition.getY() + 28);
			Bullet* bullet = new Bullet(BulletType::BOSSV);
			bullet->SetDirection(Vector2D(0, -1));
			bullet->SetParameters(Engine::GetInstance().scene.get()->configParameters);
			bullet->texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/enemies/bossBullet.png");
			Engine::GetInstance().entityManager.get()->AddEntity(bullet);
			bullet->Start();
			bullet->SetPosition(bulletPosition);
			Engine::GetInstance().audio.get()->PlayFx(shootFxId);
		}

		if (isShooting && currentAnimation->HasFinished()) {
			isShooting = false;
			shoot.Reset();
			shootD.Reset();
			currentAnimation = &idle;
		}

	}

	if (isDying) {
		b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();
		pbody->body->SetLinearVelocity(b2Vec2(0, currentVelocity.y));
	}

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), hflip);
	currentAnimation->Update();
	return true;
}

bool Boss::CleanUp() {
	if (pbody != nullptr) {
		Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	}
	return true;
}

void Boss::CreateEnemyAtPosition(Vector2D position) {
	Boss* newEnemy = new Boss();
	newEnemy->SetPosition(position);
	Engine::GetInstance().entityManager.get()->AddEntity(newEnemy);
}

void Boss::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 4);
	pos.setY(pos.getY() + texH / 4);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	if (pbody != nullptr) {
		pbody->body->SetTransform(bodyPos, 0);
	}
}

Vector2D Boss::GetPosition() {
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

Vector2D Boss::GetDirection() const {

	if (flipSprite) {
		return Vector2D(-1, 0);  // Izquierda
	}
	else {
		return Vector2D(1, 0);   // Derecha
	}
}

void Boss::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	if (pathfinding != nullptr) {
		pathfinding->ResetPath(tilePos);
	}
}

void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::BULLET:
        if (!isDying) {
			if (health > 0) {
				health--;

			}
			if (health <= 0) {
				LOG("Collided with bullet - START DYING");
				Engine::GetInstance().audio.get()->PlayFx(deathSfx);
				isDying = true;
				currentAnimation = &die;
				pbody->body->SetGravityScale(1);
			}
        }
		if (isDying && currentAnimation->HasFinished()) {
			LOG("FINISHED - DELETE BOSS");
			Engine::GetInstance().audio.get()->PlayFx(deathSfx);
			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			isDying = false;
			die.Reset();
		}
		Engine::GetInstance().entityManager.get()->DestroyEntity(physB->listener);
        break;
    case ColliderType::VOID:
        LOG("Collided with hazard - DESTROY");
        Engine::GetInstance().audio.get()->PlayFx(deathSfx);
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    }
}


void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::BULLET:
		LOG("Collision hazard");
		break;
	}
}

void Boss::SetAlive() {
	isalive = true;
	parameters.attribute("alive").set_value(true);
}

void Boss::SetDead() {
	isalive = false;
	parameters.attribute("alive").set_value(false);
}
