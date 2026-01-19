#ifndef GAMEOBSERVER_H
#define GAMEOBSERVER_H

#include "IGameObserver.h"
#include "Game.h"
#include "Constants.h"
#include <vector>

class GameObserver : public IGameObserver {
private:
	Game* game;

public:
	explicit GameObserver(Game* game) : game(game) {}

	void OnAnimationCompleted() override {
		//OutputDebugString(L"anim end\n");
	}
};

#endif



