#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Bullet.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(0, 0);
	return true;
}

bool Player::Start() {
	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	move.LoadAnimations(parameters.child("animations").child("move"));
	jump.LoadAnimations(parameters.child("animations").child("jump"));
	fall.LoadAnimations(parameters.child("animations").child("fall"));
	shoot.LoadAnimations(parameters.child("animations").child("shoot"));
	die.LoadAnimations(parameters.child("animations").child("die"));
	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), 16, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	Engine::GetInstance().scene.get()->SaveState();

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/coin.ogg");
	dieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/death.ogg");
	fallFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/fall.ogg");
	shootFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/shoot.wav");
	jumpFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/jump.wav");

	return true;
}

void Player::ResetPlayerPosition() {
	respawn = true;
	isJumping = false;
	isFalling = false;
	isDead = false;
	isShooting = false;

	currentAnimation = &idle;
	pbody->body->SetLinearVelocity(b2Vec2(0, -0.1f));
	Engine::GetInstance().scene.get()->LoadState();
	respawn = false;
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
	isRunning = false;

	position.getX();

	if (!isDead) {
		if (!isShooting) {
			// Move left
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
				flipSprite = true;
				velocity.x = -speed * 16;
				isRunning = true;
				hflip = SDL_FLIP_HORIZONTAL;
			}

			// Move right
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
				flipSprite = false;
				velocity.x = speed * 16;
				isRunning = true;
				hflip = SDL_FLIP_NONE;
			}
		}

		// Jump
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping && !isFalling) {
			Engine::GetInstance().audio.get()->PlayFx(jumpFxId);
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
			isJumping = true;
			isFalling = false;
		}

		// Fly using god mode
		if (godMode && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -0.10f), true);
			isJumping = true;
			isFalling = false;
		}

		// Shoot
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT && !isShooting && !isJumping && !isFalling) {
			isShooting = true;
			currentAnimation = &shoot;
			Vector2D bulletPosition = GetPosition();
			bulletPosition.setX(bulletPosition.getX() + (GetDirection().getX() * 28));
			Bullet* bullet = new Bullet();
			bullet->SetDirection(GetDirection());  // Establecer la direcci�n de la bala
			bullet->SetParameters(Engine::GetInstance().scene.get()->configParameters);
			bullet->texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/player/bullet.png");
			Engine::GetInstance().entityManager.get()->AddEntity(bullet);
			bullet->Start();
			bullet->SetPosition(bulletPosition);
			Engine::GetInstance().audio.get()->PlayFx(shootFxId);
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_REPEAT && !isShooting && !isJumping && !isFalling) {
			isShooting = true;
			currentAnimation = &shoot;
			Vector2D bulletPosition = GetPosition();
			bulletPosition.setY(bulletPosition.getY() - 28);
			Bullet* bullet = new Bullet();
			bullet->SetDirection(Vector2D(0, -1));  // Establecer la direcci�n de la bala hacia arriba
			bullet->SetParameters(Engine::GetInstance().scene.get()->configParameters);
			bullet->texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/player/bullet.png");
			Engine::GetInstance().entityManager.get()->AddEntity(bullet);
			bullet->Start();
			bullet->SetPosition(bulletPosition);
			Engine::GetInstance().audio.get()->PlayFx(shootFxId);
		}


		if (isShooting && currentAnimation == &shoot && currentAnimation->HasFinished()) {
			isShooting = false;
			shoot.Reset();
		}

		float verticalVelocity = pbody->body->GetLinearVelocity().y;

		// If the player is jumping, we don't want to apply gravity, we use the current velocity prduced by the jump
		if (isJumping && !isDead)
		{
			if (verticalVelocity > 0) {
				currentAnimation = &fall;
				isFalling = true;
			}
			else {
				currentAnimation = &jump;
				isFalling = false;
			}

			velocity.y = pbody->body->GetLinearVelocity().y;
		}

		// Update animation state
		if (isShooting) {
			currentAnimation = &shoot;
		}
		else if (isJumping && !isFalling) {
			currentAnimation = &jump;
		}
		else if (isFalling) {
			currentAnimation = &fall;
		}
		else if (isRunning) {
			currentAnimation = &move;
		}
		else {
			currentAnimation = &idle;
		}
	}

	if (isDead) {
		if (currentAnimation == &die && currentAnimation->HasFinished()) {
			die.Reset();
			ResetPlayerPosition();
			currentAnimation = &idle;
		}
	}

	//godmode
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		godMode = !godMode;
		if (godMode) {
			LOG("God mode on");
		}
		else {
			LOG("God mode off");
		}

	}

	//die
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
		Engine::GetInstance().audio.get()->PlayFx(dieFxId);
		isDead = true;
		currentAnimation = &die;

	}


	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(static_cast<float>(METERS_TO_PIXELS(pbodyPos.p.x)) - texH / 2);
	position.setY(static_cast<float>(METERS_TO_PIXELS(pbodyPos.p.y)) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), hflip);
	currentAnimation->Update();
	return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	float verticalVelocity = pbody->body->GetLinearVelocity().y;

	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		if (isFalling) {
			isFalling = false;
			isJumping = false;
		}
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);
		break;
	case ColliderType::HAZARD:
		if (!isDead && !godMode) {
			isDead = true;
			currentAnimation = &die;
			Engine::GetInstance().audio.get()->PlayFx(dieFxId);
			LOG("Collision HAZARD");
		}
		else{
			isJumping = false;
			isFalling = false;
		}
		break;
	case ColliderType::ENEMY:
		if (!isDead && !godMode) {
			isDead = true;
			currentAnimation = &die;
			Engine::GetInstance().audio.get()->PlayFx(dieFxId);
			LOG("Collision HAZARD");
		}
		break;
	case ColliderType::VOID:
		if (!isDead) {
			isDead = true;
			currentAnimation = &die;
			Engine::GetInstance().audio.get()->PlayFx(fallFxId);
			LOG("Collision VOID");
		}
		break;
	case ColliderType::CHECKPOINT:
		LOG("Collision Checkpoint");
		if (isFalling) {
			isFalling = false;
			isJumping = false;
		}
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		if (isFalling) {
			isFalling = false;
			isJumping = false;
		}
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		isFalling = true;
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::HAZARD:
		LOG("Collision HAZARD");
		break;
	case ColliderType::ENEMY:
		LOG("Collision ENEMY");
		break;
	case ColliderType::VOID:
		break;
	case ColliderType::CHECKPOINT:
		isFalling = true;
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Player::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

Vector2D Player::GetDirection() const {

	if (flipSprite) {
		return Vector2D(-1, 0);  // Izquierda
	}
	else {
		return Vector2D(1, 0);   // Derecha
	}
}
