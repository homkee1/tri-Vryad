#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <gdiplus.h>

enum class GameState {
	IDLE,
	SWAPPING,
	FALLING,
	MATCHING,
	REFILLING
};

const int GRID_SIZE = 16;
const int CELL_SIZE = 50;
const int MinimalGemsForType = 1;
const int MaximumGemsForType = 600;
const int GEM_TYPES_COUNT = 3;
const double SpeedAnimationA4ChangePos = 30.0;
const Gdiplus::Color COLORS[] = {
Gdiplus::Color(255, 255, 0, 0),
Gdiplus::Color(255, 0, 255, 0),
Gdiplus::Color(255, 0, 0, 255),
Gdiplus::Color(255, 255, 255, 0),
Gdiplus::Color(255, 128, 0, 128)
};

const int COLORS_COUNT = 5;

#endif