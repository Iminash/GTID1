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
    static void drawWheel(float x, float y, float z, bool isFront, bool isShadow = false);
    
    static void drawSuspension(float x, float y, float z, bool isShadow = false);
    
    static void drawCar();
    static void drawCarGeometry(bool isShadow);
    static void drawPersonAndFlash(float x, float y, float z, float seedVal, float timeMs, bool isShadow = false);
    
    static void drawTreeGeometry(const SceneryObject& obj, bool isShadow);
    static void drawBuildingGeometry(const SceneryObject& obj, bool isShadow);
    static void drawObjectGeometry(const SceneryObject& obj, bool isShadow, float timeMs);
    
    static void drawScenery();
    static void drawTrack();
};

#endif
