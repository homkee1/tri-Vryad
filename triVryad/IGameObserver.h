#ifndef IGAMEOBSERVER_H
#define IGAMEOBSERVER_H

class IGameObserver {
public:
	virtual ~IGameObserver() = default;
	virtual void OnAnimationCompleted() = 0;
};

#endif