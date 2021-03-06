/*
 * Entity.cpp
 *
 *  Created on: 2 oct. 2017
 *      Author: Valentin
 */

#include <iostream>
#include "GameController.h"
#include "entities/EntityDynamic.h"
#include "utils/Utils.h"
#include "utils/TimeManager.h"

EntityDynamic::EntityDynamic(IVector2 p) :
		EntityStatic(p), gravity(0), velocity( { 0.0, 0.0 }), onGround(false)
{

}

void EntityDynamic::tick()
{
	velocity.y += (gravity / TICK_PER_SECOND) * 1.6F;

	bool isGrounded = false;
	if (hitbox)
	{
		// X Collide
		float xVel = velocity.x;
		for (Entity* entity : GameController::getInstance().getCurrentMap()->getEntityList())
		{
			AABB* otherHitbox = entity->getAABB();
			if (otherHitbox && otherHitbox->isBlockCollision() && entity->isVisible())
			{
				otherHitbox->clipX(&velocity.x, *hitbox);
				if (xVel != velocity.x)
				{
					onCollision(entity);
					xVel = velocity.x;
				}
			}
		}

		hitbox->updatePos(m_pos.x + velocity.x, hitbox->getY());

		// Y Collide
		float yVel = velocity.y;
		for (Entity* entity : GameController::getInstance().getCurrentMap()->getEntityList())
		{
			AABB* otherHitbox = entity->getAABB();
			if (otherHitbox && otherHitbox->isBlockCollision() && entity->isVisible())
			{
				isGrounded = isGrounded || (otherHitbox->clipY(&velocity.y, *hitbox) && velocity.y >= 0);

				if (yVel != velocity.y)
				{
					onCollision(entity);
					yVel = velocity.y;
				}
			}
		}
	}

	velocity = (velocity * 0.6);
	if (velocity.magnitude() < 0.01F)
	{
		velocity = Vector2(0.0F, 0.0F);
	}

	onGround = isGrounded;
	m_realPosition = m_realPosition + velocity;
	Entity::tick();

	if (hitbox)
	{
		hitbox->updatePos(m_pos.x, m_pos.y);
	}

	// Check borders
	if (m_realPosition.x < 0)
	{
		m_realPosition.x = GameController::getInstance().getCurrentMap()->getMapWidth();
	}
	else if (m_realPosition.x >= GameController::getInstance().getCurrentMap()->getMapWidth())
	{
		m_realPosition.x = 0;
	}
	if (m_realPosition.y < 0)
	{
		m_realPosition.y = GameController::getInstance().getCurrentMap()->getMapHeight() - 1;
	}
	else if (m_realPosition.y >= GameController::getInstance().getCurrentMap()->getMapHeight())
	{
		m_realPosition.y = 0;
	}
}
