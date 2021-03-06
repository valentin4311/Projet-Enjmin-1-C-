#pragma once
#include "entities/EntityDynamic.h"

class EntityPlayer;

class EntityWeapon: public EntityStatic
{
private:
	CHAR_INFO m_charInfo;

	char m_leftSkin, m_rightSkin;

	float m_shotSpeed;
	float m_shotLifeTime;

	float m_reloadTime;
	float m_nextShotReadyTime;

	bool m_canFire;

public:
	EntityWeapon(IVector2 p, char leftSkin, char rightSkin, int shotReloadTime, float shotSpeed, float shotLifeTime);

	virtual void tick() override;

	virtual void render(BufferRenderer * buffer) override;

	void fire(IVector2 pos, EntityPlayer* shooter, int dir);

	inline char getSkins(bool right = false)
	{
		if (right)
			return m_rightSkin;
		else
			return m_leftSkin;
	}
};

