#ifndef GEM_H
#define GEM_H

#include <windows.h>
#include <gdiplus.h>
#include "Constants.h"
#include <string>
#include <mutex>

enum class GemType {
	Circle,
	Triangle,
	Pentagon
};

class Gem {
private:
	POINT pos;
	POINT targetPos;
	Gdiplus::Color color;
	GemType type;
	bool isMoving;
	LARGE_INTEGER animationStartTime;
	LARGE_INTEGER frequency;
	mutable std::mutex mtx_;

public:
	Gem(int x, int y, Gdiplus::Color c, GemType t);

	POINT getPos() const;
	bool IsMoving() const;
	Gdiplus::Color getColor() const;
	GemType getType() const;

	void ChangePos(int newX, int newY);
	void SetTargetPos(int newX, int newY, const LARGE_INTEGER& startTime);
	void ChangeColor(Gdiplus::Color newColor);

	void Update(int cellWidth, int cellHeight);

	void Draw(HDC hdc, Gdiplus::Image* image, int cellWidth, int cellHeight) const;
	void Delete();
};

#endif