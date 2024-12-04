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

Bullet::Bullet() : Entity(EntityType::BULLET)
{
	name = "bullet";
}

Bullet::~Bullet() {}

bool Bullet::Awake() {
	return true;
}

bool Bullet::Start() {

	// PROBLEMAS PARA COMENTAR CON EL PROFE
	
	/*EL LISTENER ES LA RAÍZ DE TODOS LOS PROBLEMAS

	El listener del bullet falla y se vuelve null después de borrarse. No se como solucionarlo.

	El enemy de normal después de morirse no puede ponerse con el clic, pero cuando el player se muere y respawnea, cuando clica con el ratón peta.

	Como hacer que el bullet se ponga en la posición del player y no hacerlo desde el config.xml*/


	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();


	// Cargar animaciones
	travel.LoadAnimations(parameters.child("animations").child("travel"));
	currentAnimation = &travel;

	// Agregar física al objeto - inicializar el cuerpo físico
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 4, (int)position.getY() + texH / 4, 32, 20, bodyType::STATIC);

	// Asignar tipo de colisionador
	pbody->ctype = ColliderType::BULLET;
	pbody->listener = this;


	return true;
}

bool Bullet::Update(float dt)
{
	// Aplica velocidad constante hacia la derecha
    b2Vec2 velocity = pbody->body->GetLinearVelocity();
    velocity.x = 0.0f;  // Velocidad en el eje X, ajusta según lo que necesites
    pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Dibujar al enemigo en la pantalla y actualizar su animación
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();


	return true;
}

bool Bullet::CleanUp()
{
	return true;
}

void Bullet::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 4);
	pos.setY(pos.getY() + texH / 4);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
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
		LOG("Collided with hazard - DESTROY");
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
	}
}