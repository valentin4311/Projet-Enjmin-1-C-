#include <map/Map.h>
#include "entities/EntityWeapon.h"

EntityWeapon::EntityWeapon(IVector2 p, char leftSkin, char rightSkin, int shotReloadTime, float shotSpeed, float shotLifeTime) :
		EntityStatic(p), m_leftSkin(leftSkin), m_rightSkin(rightSkin), m_shotSpeed(shotSpeed), m_shotLifeTime(shotLifeTime), m_reloadTime(shotReloadTime), m_nextShotReadyTime(0), m_canFire(true)
{
	m_charInfo.Attributes = FOREGROUND_INTENSITY;
	m_charInfo.Char.AsciiChar = rightSkin;

	hitbox = new AABB(p.x, p.y, p.x + 1, p.y + 1, true);
}

void EntityWeapon::tick()
{
	m_nextShotReadyTime--;

	if (m_nextShotReadyTime < 0)
		m_canFire = true;

	EntityStatic::tick();
}

void EntityWeapon::render(BufferRenderer * buffer)
{
	if(m_display)
	{
		buffer->setCharAt(m_pos.x, m_pos.y, m_charInfo);
	}
}

void EntityWeapon::fire(IVector2 pos, EntityPlayer* shooter, int dir)
{
	if (m_canFire)
	{
		m_canFire = false;
		m_nextShotReadyTime = m_reloadTime;
		EntityProjectile* projectile = new EntityProjectile(pos, shooter, m_shotSpeed, dir, m_shotLifeTime);
		projectile->spawn();
	}
}
