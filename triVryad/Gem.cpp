#include <cmath>
#include "Gem.h"
#include <stdio.h>
#include <string>
#include "Constants.h"
#include <windows.h>

Gem::Gem(int x, int y, Gdiplus::Color c, GemType t)
	: color(c), type(t), isMoving(false) {
	pos.x = x * CELL_SIZE;
	pos.y = y * CELL_SIZE;
	targetPos = pos;
	QueryPerformanceFrequency(&frequency);
}

POINT Gem::getPos() const {
	std::lock_guard<std::mutex> lock(mtx_);
	return pos;
}

bool Gem::IsMoving() const {
	std::lock_guard<std::mutex> lock(mtx_);
	return isMoving;
}

Gdiplus::Color Gem::getColor() const {
	std::lock_guard<std::mutex> lock(mtx_);
	return color;
}

GemType Gem::getType() const {
	return type;
}

void Gem::ChangePos(int newX, int newY) {
	std::lock_guard<std::mutex> lock(mtx_);
	pos.x = newX * CELL_SIZE;
	pos.y = newY * CELL_SIZE;
	targetPos = pos;
	isMoving = false;
}

void Gem::SetTargetPos(int newX, int newY, const LARGE_INTEGER& startTime) {
	std::lock_guard<std::mutex> lock(mtx_);
	targetPos.x = newX * CELL_SIZE;
	targetPos.y = newY * CELL_SIZE;
	isMoving = true;
	animationStartTime = startTime;
}

void Gem::ChangeColor(Gdiplus::Color newColor) {
	std::lock_guard<std::mutex> lock(mtx_);
	color = newColor;
}

void Gem::Update(int cellWidth, int cellHeight) {
	std::lock_guard<std::mutex> lock(mtx_);

	if (!isMoving) return;

	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	double elapsedTime = static_cast<double>(currentTime.QuadPart - animationStartTime.QuadPart) / frequency.QuadPart;
	double targetX = targetPos.x;
	double targetY = targetPos.y;
	double currentX = pos.x;
	double currentY = pos.y;

	double dx = targetX - currentX;
	double dy = targetY - currentY;
	double distance = sqrt(dx * dx + dy * dy);

	double timeToTarget = distance / SpeedAnimationA4ChangePos;
	float t = static_cast<float>(elapsedTime / timeToTarget);

	const double epsilon = 0.001;
	if (distance <= epsilon) {
		pos = targetPos;
		isMoving = false;
		return;
	}

	if (t >= 1.0f) {
		pos = targetPos;
		isMoving = false;
		return;
	}

	pos.x = static_cast<int>(currentX + round(dx * t));
	pos.y = static_cast<int>(currentY + round(dy * t));
}

void Gem::Draw(HDC hdc, Gdiplus::Image* image, int cellWidth, int cellHeight) const {
	Gdiplus::Graphics graphics(hdc);

	POINT safPos = getPos();
	int scaledX = safPos.x * cellWidth / CELL_SIZE;
	int scaledY = safPos.y * cellHeight / CELL_SIZE;

	if (image && image->GetLastStatus() == Gdiplus::Ok) {
		graphics.DrawImage(image, scaledX, scaledY, cellWidth, cellHeight);
	}
}

void Gem::Delete() {
	std::lock_guard<std::mutex> lock(mtx_);
	pos = { -1, -1 };
	color = Gdiplus::Color(0, 0, 0, 0);
}