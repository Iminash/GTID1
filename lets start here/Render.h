// File: Render.h
#ifndef RENDER_H
#define RENDER_H

#include "Common.h"
#include "Scenery.h"
#include "Physics.h"

class RenderSystem {
public:
    static void initTextures();
    
    static void glShadowProjection(float * l, float * e, float * n);
    
    static void drawBox(float sx, float sy, float sz);
    static void setRed();
    static void setWhite();
    static void setBlack();
    static void setYellow();
    static void drawWheel(float x, float y, float z, bool isFront);
    static void drawSuspension(float x, float z);
    static void drawCar();
    static void drawPersonAndFlash(float x, float y, float z, float seedVal, float timeMs);
    
    static void drawTreeGeometry(const SceneryObject& obj, bool isShadow);
    static void drawBuildingGeometry(const SceneryObject& obj, bool isShadow);
    
    static void drawScenery();
    static void drawTrack();
};

#endif
