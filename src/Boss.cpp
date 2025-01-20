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

    pathfinding = new Pathfinding();
    ResetPath();

    pbody->body->SetLinearVelocity(b2Vec2(0, 0));
    bossTimer.Start();
    return true;
}

void Boss::MoveTowardsTargetTile(float dt) {
    if (pathfinding == nullptr || pathfinding->pathTiles.size() < 2) return;

    //si el player está a más de 20 tiles de distancia, el enemigo se queda quieto
    if (pathfinding->pathTiles.size() > 35) {
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        return;
    }
    else {
        if (health <= 25 && health >= 0) {
            movementSpeed = angrySpeed;

            timeSinceLastAction = bossAngryTimer.ReadSec();

            if (timeSinceLastAction >= 0 && timeSinceLastAction < 2) { // Muevete a (8630, 200)
                currentAnimation = &move;
                Vector2D direction = Vector2D(8630, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
            }
            else if (timeSinceLastAction >= 2 && timeSinceLastAction < 3) { // Quedate quieto
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            else if (timeSinceLastAction >= 3 && timeSinceLastAction < 6) { // Muevete a (9300, 200) disparando
                currentAnimation = &shootD;
                Vector2D direction = Vector2D(9350, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
                Shoot();
            }
            else if (timeSinceLastAction >= 6 && timeSinceLastAction < 9) { // Muevete a (8630, 200) disparando
                currentAnimation = &shootD;
                Vector2D direction = Vector2D(8630, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
                Shoot();
            }
            else if (timeSinceLastAction >= 9 && timeSinceLastAction < 12) { // Muevete a (9300, 200) disparando
                currentAnimation = &shootD;
                Vector2D direction = Vector2D(9350, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
                Shoot();
            }
            else if (timeSinceLastAction >= 12 && timeSinceLastAction < 13) { // Quedate quieto
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            else if (timeSinceLastAction >= 13 && timeSinceLastAction < 15) { // Muevete hasta (8980, 525)
                currentAnimation = &move;
                Vector2D direction = Vector2D(8980, 525) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
            }
            else if (timeSinceLastAction >= 15 && timeSinceLastAction < 18) { // Quedate quieto
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            else if (timeSinceLastAction >= 18) { // Reinicia el ciclo
                bossAngryTimer.Start();
            }

        }
        else { // Comportamiento normal cuando salud > 25
            movementSpeed = normalSpeed;
            timeSinceLastAction = bossTimer.ReadSec();

            // Ajusta los intervalos para ralentizar el ciclo
            if (timeSinceLastAction >= 0 && timeSinceLastAction < 2) { // Intervalo extendido
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            else if (timeSinceLastAction >= 2 && timeSinceLastAction < 5) { // Movimiento 1
                currentAnimation = &move;
                Vector2D direction = Vector2D(9350, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
            }
            else if (timeSinceLastAction >= 5 && timeSinceLastAction < 6) { // Intervalo extendido
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            else if (timeSinceLastAction >= 6 && timeSinceLastAction < 9) { // Movimiento 2 + disparo continuo
                Vector2D direction = Vector2D(8630, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);

                Shoot();
            }

            else if (timeSinceLastAction >= 9 && timeSinceLastAction < 10) { // Pausa antes del siguiente movimiento
                isShooting = false;
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }

            else if (timeSinceLastAction >= 10 && timeSinceLastAction < 13) {
                Vector2D direction = Vector2D(9350, 200) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);

                Shoot();
            }
            else if (timeSinceLastAction >= 13 && timeSinceLastAction < 14) {
                isShooting = false;
                currentAnimation = &idle;
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            else if (timeSinceLastAction >= 14 && timeSinceLastAction < 17) { // Movimiento 3
                isShooting = false;
                currentAnimation = &move;
                Vector2D direction = Vector2D(8980, 525) - GetPosition();
                direction.Normalize();
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(direction.getX() * movementSpeed), PIXEL_TO_METERS(direction.getY() * movementSpeed));
                pbody->body->SetLinearVelocity(velocity);
            }
            else if (timeSinceLastAction >= 17) { // Reinicia el ciclo
                bossTimer.Start();
            }
        }
    }
    

    
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

        // Actualiza flipSprite solo cuando cambie significativamente la dirección
        float velocityX = pbody->body->GetLinearVelocity().x;
        if (velocityX < -0.1f && !flipSprite) {
            flipSprite = true;
            hflip = SDL_FLIP_HORIZONTAL;
        }
        else if (velocityX > 0.1f && flipSprite) {
            flipSprite = false;
            hflip = SDL_FLIP_NONE;
        }
    }

    if (currentAnimation == &shootD && currentAnimation->HasFinished()) {
        isShooting = false;
        shootD.Reset();
    }

    if (isDying) {
        b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();
        pbody->body->SetLinearVelocity(b2Vec2(0, currentVelocity.y));
    }

    // Actualiza posición y renderiza
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

void Boss::Shoot()
{
    // Temporizador para disparos
    if (!isShooting) { // Dispara cada 500 ms
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
        isShooting = true;
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
