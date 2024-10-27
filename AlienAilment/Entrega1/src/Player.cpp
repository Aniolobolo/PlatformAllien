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
	hit.LoadAnimations(parameters.child("animations").child("hit"));
	die.LoadAnimations(parameters.child("animations").child("die"));
	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), 16, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/coin.ogg");
	dieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/death.ogg");
	fallFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/fall.ogg");

	return true;
}

void Player::ResetPlayerPosition() {
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY())), 0);

	isJumping = false;
	isFalling = false;
	isDead = false;

	currentAnimation = &idle;
	pbody->body->SetLinearVelocity(b2Vec2(0, -0.1f));
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
	isRunning = false;

	if (!isDead) {
		// Move left
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			flipSprite = true;
			velocity.x = -speed * 16;
			isRunning = true;
			if (flipSprite == true && hflip == SDL_FLIP_NONE) {
				hflip = SDL_FLIP_HORIZONTAL;
			}
		}

		// Move right
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			flipSprite = false;
			velocity.x = speed * 16;
			isRunning = true;
			if (flipSprite == false && hflip == SDL_FLIP_HORIZONTAL) {
				hflip = SDL_FLIP_NONE;
			}

		}

		float verticalVelocity = pbody->body->GetLinearVelocity().y;
		//Jump
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false) {
			// Apply an initial upward force
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
			isJumping = true;
			isFalling = false;
		}

		//fly using god mode
		if (godMode && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -0.15), true);
			isJumping = true;
			isFalling = false;
		}

		if (isRunning) {
			currentAnimation = &move;
		}
		else {
			currentAnimation = &idle;

		}

		if (!isJumping && isFalling) {
			currentAnimation = &fall;
		}

		// If the player is jumping, we don't want to apply gravity, we use the current velocity prduced by the jump
		if (isJumping == true && !isDead)
		{
			if (verticalVelocity > 0) {
				currentAnimation = &fall;
				isFalling = true;
			}
			else {
				currentAnimation = &jump;
			}

			velocity.y = pbody->body->GetLinearVelocity().y;
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
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

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
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		isFalling = false;
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
		break;
	case ColliderType::HAZARD:
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
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
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
	case ColliderType::VOID:
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}
