#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <windows.h>
#include <algorithm>
#include <string>
namespace Gdiplus {
    using std::min;
    using std::max;
}

#include <gdiplus.h>

enum class GameState {
    IDLE,
    SWAPPING,
    FALLING,
    MATCHING,
    REFILLING
};

extern int GRID_SIZE;
extern int CELL_SIZE;
extern int GEM_TYPES_COUNT;
extern double SpeedAnimationA4ChangePos;
extern std::wstring PNG_FOLDER_PATH;
extern int RENDERING_QUALITY; // 0 низкое, 1 среднее, 2 высокое
extern int GEM_IMAGE_SIZE;    // физическое разрешение текстуры в памяти

const int MinimalGemsForType = 1;
const int MaximumGemsForType = 600;

inline Gdiplus::Color GetColorForType(int type) {
    const int PREDEFINED_COLORS_COUNT = 10;
    static const Gdiplus::Color PREDEFINED_COLORS[PREDEFINED_COLORS_COUNT] = {
        Gdiplus::Color(255, 255, 0, 0),       // красный
        Gdiplus::Color(255, 0, 255, 0),       // зеленый
        Gdiplus::Color(255, 0, 0, 255),       // синий
        Gdiplus::Color(255, 255, 255, 0),     // желтый
        Gdiplus::Color(255, 128, 0, 128),     // фиолетовый
        Gdiplus::Color(255, 255, 128, 0),     // оранжевый
        Gdiplus::Color(255, 0, 255, 255),     // голубой
        Gdiplus::Color(255, 255, 0, 255),     // хз
        Gdiplus::Color(255, 128, 128, 128),   // серый
        Gdiplus::Color(255, 255, 255, 255)    // белый
    };

    if (type >= 0 && type < PREDEFINED_COLORS_COUNT) {
        return PREDEFINED_COLORS[type];
    }
    return Gdiplus::Color(255, (type * 45) % 256, (type * 75) % 256, (type * 115) % 256);
}

#endif