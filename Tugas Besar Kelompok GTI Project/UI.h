#ifndef UI_H
#define UI_H

#include "Common.h"
#include <string>

class UISystem {
public:
    static float totalTime;
    static std::string formatTime(float t);
    static void drawText(int x, int y, std::string s, void* font = GLUT_BITMAP_HELVETICA_18);
    static void render();
};

#endif
