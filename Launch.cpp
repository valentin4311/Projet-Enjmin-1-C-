#include <stdio.h>
#include <iostream>

#include "./include/Map.h"
#include "./include/NYTimer.h"

bool shallClose;
void loopGame();

int main(int argc, char* args[])
{
	loopGame();

	return 0;
}

void loopGame()
{
	NYTimer timer;

	Player test(IVector2(2,2));
	test.spawn();


	while(!shallClose)
	{
		float delta = timer.getElapsedMs();

		Map::getMap().update(delta);
		Map::getMap().drawBuffer();

		shallClose = GetAsyncKeyState(VK_ESCAPE);
	}
}
